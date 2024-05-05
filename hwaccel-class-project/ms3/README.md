# Milestone 3: Improved Hardware-Software Interface
In this third milestone, we will now improve the performance of the
hardware-software interface to (hopefully) finally beat the software baseline.
We will use two techniques: reduce the amount of communication needed, and
implement the necessary communication more efficiently.

## Directory Structure
* `Makefile`: Makefile to build, run, and test everything.
* `app/`: Application code we are running for our measurements. Runs inside the
  simulated machine.
  + `app/matmul-accel.c`: Simple matrix multiplication test application, uses
    the driver below to issue operations to the accelerator.
    *No need to modify this in ms3.*
  + `app/driver.c`: User-space driver for the hardware accelerator.
* `common/reg_defs.h`: Register definitions for the accelerator PIO interface.
   Used by simulation model and driver.
* `accel-sim/`: Simulation model for the HW accelerator.
  + `accel-sim/plumbing.c`: Simulation harness and implementation of the
    SimBricks integration. No need to modify this.
  + `accel-sim/dma.c`: DMA engine implementation. No need to modify this.
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

## Step 1: Aggregate Outputs for Block Multiplication On-device

In our last milestone we implemented block multiplication with the accelerator.
It turns out, much of the time is actually spent copying the intermediate output
matrices from the accelerator buffer to host memory. So the first step is to
reduce the number of output matrices transferred from the accelerator to the
host by doing the summation of all elements for one block-output on the
accelerator. When performing a block matrix multiplication, you will now only
copy the inputs in for all `B` sub-matrices you need for an individual output
block, and only then retrieve the output once.

We suggest that you implement this with minimal changes to the interface, by
simply adding a flag to the accelerator that controls if the operation should
reset the output buffer to zero or just add into it:

```
/** Control register: used to start the accelerator/wait for completion.
    read/write */
#define REG_CTRL 0x08
/** Run bit in the control register, set to start, then wait for device to clear
    this back to 0.*/
#define REG_CTRL_RUN 0x01
/** Reset bit for output buffer. When set, clears the output buffer to 0. */
#define REG_CTRL_RSTOUT 0x02
```

The driver can then set this for the first operation for an output block, and
then just perform the rest with the bit cleared and finally retrieve the output.

You will need to implement support for this in the simulation model as well as
the driver.

Test 0 is a functional test testing for the correctness of the multiplication
result:
```
$ make test0.out
...
host.host OUT: ['STATUS: Success matrices match\r']
...
TEST test0
SUCCESS test0
```

And test 1 is a performance test that should show significantly reduced
cycles per op compared to the prior baseline:
```
$ make test1.out
...
TEST test1
Delay 10000000000 -> 729319637 Cycles/op
Delay 1000000000 -> 152685450 Cycles/op
Delay 1000000 -> 88688202 Cycles/op
Delay 1000 -> 88612925 Cycles/op
SUCCESS test1
```

## Step 2: Reduce Repeated Copies of Input Matrices

Next, we will further reduce redundant communication during block
multiplication. Specifically we notice that for calculating all blocks for a row
of the output matrix, the same row of input A blocks are used. So if we can
store a whole row of input A on the accelerator, we can avoid much unnecessary
communication.

To achieve this we will increase the amount of memory on the accelerator and
make the management and use of it more flexible. Instead of having fixed buffers
for the inputs and outputs, we will now have a larger flexible pool of memory.
The driver then manages this memory and decides which matrices to place where,
and for each multiplication first specifies to the accelerator which addresses
in that memory to use. Thereby the driver can keep inputs or outputs cached on
the device flexibly.

Your task for step 2 is to implement this in the hardware simulator and the
driver. The amount of on-device memory will be a command line parameter to the
hardware simulator, and **your driver should fall back to the slower approach if
there is not enough space for the whole row of blocks**, otherwise the earlier
tests will fail.

Here is a suggested hardware interface for this:


| Offset   | Description                                                       |
|----------|-------------------------------------------------------------------|
| `0x10`   | `RW`: `OFF_INA`. Offset of the input A buffer in the on-device memory.|
| `0x18`   | `RW`: `OFF_INB`. Offset of the input B buffer in the on-device memory.|
| `0x20`   | `RW`: `OFF_OUT`. Offset of the output buffer in the on-device memory.|
| `0x30`   | `RO`: `MEM_SIZE`. Size of the on-device memory in bytes           |
| `0x38`   | `RO`: `MEM_OFF`. Offset in the register region to access the on-device memory. |
| `OFF_MEM`| Buffer for matrices, managed by driver, and used on the accelerator via `OFF_INA/INB/OUT`|


Test 2 is a functional test testing for the correctness of the multiplication
result (note unlike test 0, this will specify exactly enough memory for a row of
blocks, second input block, and output block):
```
$ make test2.out
...
host.host OUT: ['STATUS: Success matrices match\r']
...
TEST test2
SUCCESS test2
```

And test 3 is a performance test that should show further reduced
cycles per op compared to step 1:
```
$ make test3.out
...
Delay 10000000000 -> 710492261 Cycles/op
Delay 1000000000 -> 133937775 Cycles/op
Delay 1000000 -> 69963135 Cycles/op
Delay 1000 -> 69887947 Cycles/op
SUCCESS test3
```

## Step 3: Minimally Optimizing Communication with DMA

Now that we have reduced redundant communication, it is time to optimize the
communication that we do have to do. And the way we do this is through direct
memory access (DMA), i.e. having the accelerator directly load and store data
in memory, rather than poking it in and out through small register operations.
This is a common hardware technique used by most modern devices.

This will again require changes to your hardware simulator and driver. First
off, you will need to design an interface for the driver to request DMA
transfers. We have included a suggested design for the registers. Basically,
the driver first writes the length, **physical** memory address, and offset in
on-device memory into the corresponding registers. Then the driver writes the
run bit to the DMA control register, along with a potentially set `W` bit to
indicate a write rather than a read if `W` is cleared:

```
/** DMA Control register. */
#define REG_DMA_CTRL 0x40
/** Bit to request start of DMA operation */
#define REG_DMA_CTRL_RUN 0x1
/** Bit to request DMA Write (device to host), if 0 the operation is a read
 * instead (host to device). */
#define REG_DMA_CTRL_W   0x2

/** Register holding requested length of the DMA operation in bytes */
#define REG_DMA_LEN  0x48
/** Register holding *physical* address on the host. */
#define REG_DMA_ADDR 0x50
/** Register holding memory offset on the accelerator. */
#define REG_DMA_OFF  0x58
```

Note that for DMA you will need **physical addresses**. For this we provide a
special allocator function and pre-allocate a chunk of physically contiguous
memory mapped into the application and with a known physical address. See
`dma_mem*`. You will have to copy data via this DMA memory in software,
unfortunately. This is a common problem with drivers.

In the driver you can then implement these DMA transfers in the driver, for
copying matrices to and from the device. **Make sure to only use DMA if the
`use_dma` flag is set, otherwise you should fall back to PIO for the tests in
the previous**.

**Caution:** Due to a bug in the gem5 simulator you should avoid using the same
physical DMA buffer address for DMA reads and writes. Given that you will only
use the DMA buffers for intermediate copies, this is easy to implement. In the
skeleton we provide `dma_mem`/`dma_mem_phys` and `dma_mem_w`/`dma_mem_phys_w`,
respectively. If you use the same buffer, you will see panics in gem5.

### Performing DMA

Despite what the name implies, in the device simulator (just as in real hardware)
you cannot directly access host memory. Instead the device has to issue
asynchronous DMA read/write requests on the PCIe interconnect. The device issues
these, and then at some later point gets a response from the host system. This
is only direct in so far as it does not involve the processor. The interconnect
also places limits on the maximum size of an individual access (on typical PCIe
systems this is somewhere between 512 and 4096 bytes). For larger transfers the
device has to issue multiple smaller accesses to subsequent addresses.

For the simulator in this milestone we provide the following two API calls you
can use to issue a DMA operation (see `sim.h`). **Note** these calls *do*
support larger DMAs and will internally take care of turning this into multiple
smaller operations for you. But they are still asynchronous.

```
/**
 * Issue a DMA Read operation (read data from host memory).
 * @param dst      Destination pointer in the simulator
 * @param src_addr *Physical* address on the host.
 * @param len      Number of bytes to read.
 * @param opaque   Value passt back to `DMACompleteEvent` when called on
 *                 completion.
 */
void IssueDMARead(void *dst, uint64_t src_addr, size_t len, uint64_t opaque);

/**
 * Issue a DMA Write operation (write data to host memory).
 * @param dst_addr *Physical* address on the host.
 * @param src      Source pointer in the simulator
 * @param len      Number of bytes to write.
 * @param opaque   Value passt back to `DMACompleteEvent` when called on
 *                 completion.
 */
void IssueDMAWrite(uint64_t dst_addr, const void *src, size_t len,
    uint64_t opaque);
```

Once the full DMA operation completes, you will get a callback to the
`DMACompleteEvent` function in your code. If you need multiple parallel DMA
operations, you can use the `opaque` parameter in the `IssueDMA` functions, and
it will be passed through to the completion callback to tell you which operation
completed:

```
/** Called when a DMA operation is completed. Opaque value is the one passed in
 * on DMA issue. */
void DMACompleteEvent(uint64_t opaque);
```

### Testing
Test 4 is a functional test testing for the correctness of the multiplication
result with DMA enabled:
```
$ make test4.out
...
host.host OUT: ['STATUS: Success matrices match\r']
...
TEST test4
SUCCESS test4
```

And test 5 is a performance test that should show again significantly faster
multiplies:
```
$ make test5.out
...
TEST test5
Delay 10000000000 -> 655449609 Cycles/op
Delay 1000000000 -> 79425909 Cycles/op
Delay 1000000 -> 15451757 Cycles/op
Delay 1000 -> 15376439 Cycles/op
SUCCESS test5
```
