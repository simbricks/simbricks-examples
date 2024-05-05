# Milestone 2: Behavioral Model for Basic Accelerator with PIO Interface
In the second project milestone we will now actually start to look at hardware
acceleration. As a first step we will build a simple behavioral simulator for
the accelerator, where we can plug in different parameters for size and
performance to experiment what makes sense. You will design and implement a
simple PIO register interface for the accelerator and then fully integrate
the accelerator into our test application with a user space driver for it. In a
second step you will extend the driver. You can
then measure performance for the application with this acceleration architecture
(spoiler: it will not be good ;-)).

The background to this is, that this will allow us to explore high-level design
parameters for the system: size of the matrix multiply unit, and the time the
accelerator takes to complete an operation. The simulation allows us to explore
the effect of these parameters on our workload application performance.

## Directory Structure
* `Makefile`: Makefile to build, run, and test everything.
* `app/`: Application code we are running for our measurements. Runs inside the
  simulated machine.
  + `app/matmul-accel.c`: Simple matrix multiplication test application, uses
    the driver below to issue operations to the accelerator.
    *No need to modify this in ms2.*
  + `app/driver.c`: User-space driver for the hardware accelerator.
* `common/reg_defs.h`: Register definitions for the accelerator PIO interface.
   Used by simulation model and driver.
* `accel-sim/`: Simulation model for the HW accelerator.
  + `accel-sim/plumbing.c`: Simulation harness and implementation of the
    SimBricks integration. No need to modify this.
  + `accel-sim/sim.c`: Actual accelerator simulation model logic.
* `tests/`: configurations for all the tests.
  + `app/test*.sim.py`: Simulation configuration
  + `app/test*.check.py`: Automated check of simulation result
* `out/`: Outputs from completed simulation runs.

## Simulations & Tests
As before, the simulations can be run with `make test`. This will sequentially
run all of them. Expect this to take around 20 minutes. If you have enough cores
and memory you can parallelize them with `make test PARALLEL=y`. Note that this
will already use 8 cores, so we would suggest not using `-jN` on top of this,
unless you have at least 16 cores. With full parallelization the longest test
will take around 3 minutes. You can run individual tests with `make testN.out`.

`make check` will validate all test outputs, if they exist or print failures
otherwise. After running `make test` or running tests individually `make check`
should print success for each test.

## Step 1: Design PIO Register Interface for Accelerator
The first step for this milestone is to decide on a register interface for the
hardware accelerator. Basically the question to answer is: How should the
application interact with the accelerator.

We provide a suggestion for a basic interface in `common/reg_defs.h`:

| Offset   | Description                                                       |
|----------|-------------------------------------------------------------------|
| `0x00`   | `RO`: Accelerator Matrix Size. Size (width) of matrices the accelerator supports.
| `0x08`   | `RW`: Control register. Only bit 0 is used. If set by the host, the accelerator starts executing an operation. When completed, the accelerator will clear the bit. |
| `0x10`   | `RO`: `OFF_INA`. Contains the offset of the input A buffer in the register space. |
| `0x18`   | `RO`: `OFF_INB`. Contains the offset of the input B buffer in the register space. |
| `0x20`   | `RO`: `OFF_OUT`. Contains the offset of the output buffer in the register space. |
| `OFF_INA`| Buffer for input A, the left input matrix.                        |
| `OFF_INB`| Buffer for input B, the right input matrix.                       |
| `OFF_OUT`| Buffer for the output result matrix.                              |

With this interface, the driver will first read out the size register to find
out the supported matrix size. Next it reads the `OFF_*` registers to locate the
input and output buffer offsets. Then the driver can copy the input matrices
into the two input buffers through register writes to the offsets starting at
the values read from the registers. Next the driver would set the run bit in the
control register with a write. Now, the driver repeatedly reads the control
register back until the run bit is set to 0 again, at which point it knows the
operation is complete. Finally, the driver reads the output from the output
buffer through register reads.

**Note that this interface is just a suggestion. You can change this as you see
fit.** If you change it, you should also modify `common/reg_defs.h` so you have
macros for the register offsets you can use in the driver and simulation model.

## Step 2: Implement & Test Your First Register
After deciding on an interface, the next step is to implement enough
functionality in the simulation model in `accel-sim/sim.c` (`MMIORead` function)
and the driver in `app/driver.c` (`accelerator_matrix_size` function), to allow
the driver to detect the supported matrix size. You only need to modify these
two files for this, no changes to the application code and the plumbing code for
the simulator are necessary.

After this, the first test 0 will pass. `make test0.out` will run the first
functional test with two different size configurations for the accelerator. If
the driver detects these correctly, the test will pass:
```
test0-128: started dev.host.accel
test0-128: starting host.host
...
host.host OUT: ['+ /tmp/guest/matmul-accel\r']
host.host OUT: ['Accelerator Size: 128\r']
...
test0-512: started dev.host.accel
test0-512: starting host.host
...
host.host OUT: ['+ /tmp/guest/matmul-accel\r']
host.host OUT: ['Accelerator Size: 512\r']
...
SUCCESS test0
```

## Step 3: Complete Simulation Model
After this works, you can now complete the simulator model for the accelerator
in `accel-sim/sim.c`. Note that the header `accel-sim/sim.h` has more
description on the individual functions. Basically all of those functions are
callbacks from the main simulation loop. In `InitState()` you can initialize
any internal state you need for your simulation, e.g. allocate buffers for
matrices etc. `MMIORead()` and `MMIOWrite()` are called whenever a register is
read or written, and you can implement functionality to handle these events. We
provide scaffolding and an example there.

The simulator has two command-line parameters: the size (width) of the matrix
`matrix_size` and the latency for completing a multiplication `op_latency`.

The simulation is a discrete event simulation. Basically, it keeps a queue of
events along with timestamps. All of this is already implemented for the PCIe
interface, so you will get calls for MMIO operations etc. when the simulation is
at the corresponding time. You only need to implement corresponding hooks for
other events.

Specifically, there is one new event you need to add: the completion of the
multiplication after it is started through a register write. When the register
write arrives, you can read the current simulation time (in picoseconds) from
`main_time`, and calculate when it should complete (`main_time` + `op_latency`).
Whenever this event is pending, i.e. a multiplication is ongoing, you must
return the timestamp that event should execute in `NextEvent()` (otherwise
return `UINT64_MAX`), this will guarantee that the simulation time will not
advance beyond that timestamp without first calling `PollEvent()`, at which
point you can complete the event, e.g. put the result in the output buffer and
clear the flag for future register reads.

Note that the simulation loop will periodically call `PollEvent()` either way,
and you have to handle calls correctly, whether there is an event or not.
`NextEvent()` just ensures that no event timestamp is missed.

The reference implementation extends the existing `sim.c` to 172 lines total.

## Step 4: Implement the Basic Driver
Now that the simulation model is complete, it is time to implement the driver in
`app/driver.c`. We already provide the plumbing code to memory map the registers
in user space through the Linux `vfio-pci` kernel driver (see
`../common/vfio-pci.c` if you are curious). For this you may have to extend the
`accelerator_init()` function, and then implement the `matmult_accel()`
function. **Note that for this step you can assume (but check) that the matrix
size matches the accelerator.**

We also provide an example for how to read a register with the `ACCESS_REG()`
macro. Note you can also write a register with `ACCESS_REG(FOO) = bar` (the main
purpose is to ensure the volatile attribute to prevent the compiler from
optimizing register accesses away).

The reference implementation extends the existing `driver.c` with about 40 lines
for this.

## Step 5: Test Functionality & Performance for Single Operation
Tests 1 and 2, validate the correctness and performance of the complete system.
Test 1 compares the result of an accelerated multiplication to the software
baseline:

```
$ make test1.out
...
host.host OUT: ['+ /tmp/guest/matmul-accel 512\r']
host.host OUT: ['STATUS: Success Matrices match\r']
```

Once this passes, test 2 will do some performance tests of the system with
different operation times for the multiplication, ranging from 1ns to 10ms:
```
$ make test2.out
TEST test2
Delay 10000000000 -> 61002611 Cycles/op
Delay 1000000000 -> 52002860 Cycles/op
Delay 1000000 -> 51003057 Cycles/op
Delay 1000 -> 51001909 Cycles/op
SUCCESS test2
```

Note that test 2 will not report success unless test 1 passes. The success
criterion for the performance is that the operation with 10ms latency take at
least 8ms longer than the one with 1ns latency per matrix. This is to test that
your simulation model correctly models the delay.

## Step 6: Extend your Driver for Larger Matrices
Next you should extend your driver to support multiplications of matrices larger
than the accelerator supports in hardware. This will allow us to determine how
large the accelerator needs to be in relation to its cost later on. The idea is
to leverage a block matrix multiplication where the block size is the supported
hardware matrix size, and then assemble the complete multiplication from
multiple of these smaller operations. Implement this in `matmult_accel` with a
check for an `n` larger than the supported matrix size.

The reference implementation extends the existing `driver.c` to about 140 lines
total.

## Step 7: Test functionality and Performance for Larger Matrices
Once implemented, tests 3 and 4 will validate functionality and performance. The
test scripts will aim to perform a 512x512 multiplication with an accelerator
supporting 128x128 multiplications.

Test 3 will again test the result against the software baseline:
```
$ make test3.out
...
host.host OUT: ['+ /tmp/guest/matmul-accel 512\r']
host.host OUT: ['STATUS: Success Matrices match\r']
```

And test 4 will again check for performance with operation latencies ranging
from 1ns to 10ms. Note that a 512x512 multiplication will take 4x4x4 = 64
multiplication of size 128x128. As a result a successful test will expect a
difference of at least 80% of 640ms between the case with 1ns operation time and
the one with 10ms:
```
$ make test4.out
...
Delay 10000000000 -> 847562585 Cycles/op
Delay 1000000000 -> 270723885 Cycles/op
Delay 1000000 -> 206637001 Cycles/op
Delay 1000 -> 206553849 Cycles/op
SUCCESS test4
```
