# Milestone 5: Hardware RTL Design
In this fifth (including the bonus milestone) milestone, we will now look at how
to actually implement the hardware accelerator, quantify the hardware cost,
 examine what accelerator performance is feasible, and explore these trade-offs.



## Directory Structure
* `Makefile`: Makefile to build, run, and test everything.
* `app/`: Application code we are running for our measurements. Runs inside the
  simulated machine. In this milestone, you do not have to modify this.
  + `app/matmul-accel.c`: Simple matrix multiplication test application, uses
    the driver below to issue operations to the accelerator.
  + `app/driver.c`: User-space driver for the hardware accelerator.
* `common/reg_defs.h`: Register definitions for the accelerator interface.
   Used by simulation harness and driver.
* `accel-sim/`: Simulation harness for the RTL HW accelerator. No need to modify
  this, except possibly for enabling debugging and tracing.
  + `accel-sim/plumbing.c`: Simulation harness and implementation of the
    SimBricks integration. 
  + `accel-sim/dma.c`: DMA engine implementation. No need to modify this.
  + `accel-sim/sim.cpp`: Integration with the verilog simulation.
* `hw_comb/`: Simple combinatorial RTL implementation of a matrix multiplication
  + `hw_comb/top.v`: Top-level module for the accelerator.
  + `hw_comb/matmul_comb.v`: Fully combinatorial RTL implementation of a matrix
    multiplication.
* `hw_vec/`: Sequential RTL implementation based on sequential calculation of
  output cells with a parallel vector dot-product.
  + `hw_vec/top.v`: Top-level module for the accelerator.
  + `hw_vec/matmul_vec.v`: Sequential RTL implementation of the matrix multiply.
    Calculates one element of output matrix per cycle. Instantiate fetch and
    calc stages.
  + `hw_vec/matmul_fetch.v`: Fetch stage for sequential matrix multiplication.
  + `hw_vec/matmul_calc.v`: Calculation stage for sequential matrix
    multiplication.
+ `hwresults`: Data from and scripts for hardware synthesis. 
* `tests/`: configurations for all the tests.
  + `app/test*.sim.py`: Simulation configuration
  + `app/test*.check.py`: Automated check of simulation result
* `out/`: Outputs from completed simulation runs.

## Simulations & Tests
As before, the simulations can be run with `make test`. This will sequentially
run all of them. Expect this to take around 10 minutes. You can run individual
tests with `make testN.out`.

`make check` will validate all test outputs, if they exist or print failures
otherwise. After running `make test` or running tests individually `make check`
should print success for each test.

## Step 1: Understand and Run the Combinatorial Implementation

We start with a simple, fast but very expensive matrix multiplication
implementation: a fully combinatorial circuit that implements a full matrix
multiplication in just one clock cycle. You can find the code for this in
`hw_comb/`. `top.v` is the top-level module that describes the overall design.
Think of the inputs and outputs of these modules as connecting to the pins on
the chip you build. The top module contains the logic to interact with the
outside system through MMIO and DMA (although the actual dma logic is assumed to
be implemented in external logic). `matmul_comb.v` contains the actual
implementation of the matrix multiplication logic.

Make sure you understand the verilog code before moving on to the next steps.

`test0` is a functional test simulation of the complete system running the
complete verilog and using this for the multiplication:

```
$ make test0.out
...
host.host OUT: ['STATUS: Success matrices match\r']
...
TEST test0
SUCCESS test0
```

## Step 2: Implement Sequential RTL Matrix Multiplication

Next, we will implement a more space-efficient partially sequential hardware
matrix multiplication. Instead of implementing a full combinatorial circuit for
a full matrix multiplication, we will only implement a vector dot product in
parallel, thereby reducing the spatial complexity for the computation from
`O(n^3)` to `O(n)`. Instead, the computation will now take `O(n^2)` cycles, as
we will calculate one element of the output matrix per cycle.

For this the accelerator will store the matrices in memories that contain one
full vector per address. Each cycle the multiplier will fetch the next
combination of input vectors for both matrices. Every `N` cycles it will write
out an output vector. *To make things easier, we rely on the driver to transpose
the second input matrices, so we can retrieve consecutive row vectors from both
matrices.*

Fetching an entry from memory has a latency of 1 cycle. Thus a pipelined
architecture makes sense: in the first stage we fetch the vectors, and in the
next stage we multiply them and potentially initiate the write to the output
matrix. We implement these stages in separate modules: `mul_vec.v` is the
overall module for the multiplier and instantiates `mul_fetch.v` and
`mul_calc.v` to perform the operation. The top-level module starts the
sequential multiplier by providing it with a `start` signal, and eventually the
multiplier will signal completion by asserting the `done` signal, both only for
one cycle.

The skeleton provides the complete top modules and also the complete `mul_vec`
module definitions, but you will have to fill in the gaps in the `mul_fetch` and
`mul_calc` modules (Look for the `FILL ME IN` comments). You are welcome to
modify other parts as well, but you do not need to do so. In the reference
solution these modules have a total of 129 and 104 lines, respectively,
including comments and empty lines.

You can test your implementation with `test1`, which will execute a full
simulation with the accelerator and compare its result to the software
implementation:

```
$ make test1.out
...
host.host OUT: ['STATUS: Success matrices match\r']
...
TEST test1
SUCCESS test1
```

Note that we keep the accelerator size small (8-32) to keep processing time in
the next steps manageable.

### Debugging Verilog

You are unlikely to end up with a fully correct implementation on your first
try. Unfortunately, debugging hardware in general gets quite hairy. One of the
nice things when relying on simulation is that we can record a great level of
detail to examine after a run. Our simulation harness has recording of signal
traces enabled by default. This means that after every simulation run you will
find a file `out/debug.vcd`, that contains the **full trace of each signal in
the verilog design over the complete simulation time**. These traces, or
*waveforms* as they are called in the HW world, are a very powerful debugging
tool. You can open the vcd file with a waveform viewer, such as
[gtkwave](https://gtkwave.sourceforge.net/) (available in most distros' package
managers). Unfortunately we cannot ship this as part of the docker image.

In the waveform viewer you can select which signals you are interested in and
then observe how they change at each cycle over time and in relation to
each other. The tools typically also make it easy to navigate to places where a
signal is changing, to help you find regions of interest.


## Optional Step 4: Examine HW cost and Performance

In verilog, as in simulation, we can implement just about anything. But that
does not mean that such a system can actually be realized in hardware, or at
least realized at that performance. To figure out the costs and feasible
performance of a hardware design, we have to go a step further, and actually
make our way closer to a full hardware implementation.

For this, we will now *synthesize* our RTL design as a 130nm chip. This will
give us a lot of information about HW cost and performance. However, this is
also a bit of a hairy and slow process. And so you are welcome to skip the step
of doing this yourselves, as we have included the results in the repository.
But we also include the necessary steps and scripts for you to reproduce this
and play with the tools.

Specifically we are targeting the open source Sky 130nm process node, and use
the open source [OpenROAD](https://theopenroadproject.org/) /
[OpenLane](https://openlane.readthedocs.io/en/latest/) chip design tools. For
the purposes of this milestone, we are interested in taking our design from RTL
through synthesis, static timing and power analysis, and floor planning. This
will give us information on feasible clock frequencies/periods, power
consumption, and required chip area.

Here are the pre-computed results for the vector and combinatorial design, with
different sizes and clock periods:

```
$ cat hwresults/vec.csv 
size,period,wns,power,area
8,8,0.00,0.00927,0.22968028160000004
8,9,0.00,0.00824,0.22968028160000004
8,11,0.00,0.00674,0.22968028160000004
16,8,-0.82,0.0344,0.8117285120000001
16,9,0.00,0.0306,0.8117285120000001
16,11,0.00,0.025,0.8117285120000001
32,8,-2.29,0.132,3.0900360896
32,9,-1.29,0.117,3.0900360896
32,11,0.00,0.096,3.0900360896

$ cat hwresults/comb.csv
size,period,wns,power,area
8,8,-0.38,0.00918,2.3296831008
8,9,0.00,0.00816,2.3296831008
16,9,-0.84,0.0293,18.0778030464
16,10,0.00,0.0264,18.0778030464
```

The `size` column indicates the size of the matrix multiplier (8x8,16x16,32x32).
`period` is the clock period in nanoseconds, so we are seeing clock frequencies
~100MHz. `wns` is the worst negative slack, when negative this indicates by how
many nano seconds the longest critical path in the design missed the necessary
timing target; 0 indicates that clock period can be implemented, a negative
value means it will not work. `power` is a rough estimation for how many watts
of power the chip will need at that frequency. Finally `area` is the chip area
in square millimeters.

As you can see, the sequential vector design is a lot more compact in terms of
area, needs slightly more power, and supports slightly faster clocks.

### Obtaining these Numbers Yourself

If you would like to obtain these numbers yourself, here are the steps. First,
note that this will require downloading a few gigabytes worth of docker images
technology libraries. Because of the size, we did not include this in the main
devcontainer image. So to obtain the numbers, you need to start by switching
your devcontainer image and rebuilding & restarting the container in vscode:

```
$ cp hwresults/devcontainer.json ../../.devcontainer.json
```

After this, vscode should prompt you to rebuild the container. This will take a
few minutes even with a fast internet connection. Once vscode has restarted the
container, you are ready to begin.

To obtain the results for the sequential vector version:
```
$ cd hwresults
$ ln -s ../hw_vec src
$ rm -rf runs
$ bash explore.sh
$ python3 parse.py
size,period,wns,power,area
8,8,0.00,0.00927,0.22968028160000004
...
```
Overall this probably takes 15-20 minutes.

You can proceed analogously for the combinatorial design. Note that this will
take a lot longer (a few hours).

Afterwards, remember to switch back to the original devcontainer and rebuild:
```
git checkout ../../.devcontainer.json
```

# Step 5: Examine End-to-End System Performance

Now that we have established how fast the designs can run in hardware and what
the area and power costs are, we can now examine overall system performance of
different designs. `test2` compares the combinatorial and sequential multipliers
by measuring the full software multiplication request time:

```
$ make test2.out
...
TEST test2
HW comb with 9ns/clk results in 19207 cycles/op
HW vec with 8ns/clk results in 20718 cycles/op
SUCCESS test2
```

As you can see, the combinatorial design that completes the compute in just one
cycle is in the end only 8% faster, despite being about an order of magnitude
larger chip area!
