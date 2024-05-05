# Milestone 1: Software Baseline
In the first project milestone we will work on the software baseline, that we
use for comparison in later milestones. This serves partially as an opportunity
to ensure that the tools and environment is fully working for you.

For the series of milestones in course, we will use a conceptually simple but
computationally dense task: matrix multiplication. We will use full system
simulations with the [SimBricks](https://simbricks.github.io/) framework for our
evaluation. While we could evaluate the first milestone on real hardware, this
will not be possible for future milestones. And since we need a meaningful
performance baseline for an apples-to-apples comparison, we will already start
in simulation now. An additional benefit is that this will result in
reproducible results, regardless of who is running the measurements or on which
machine.

*Note that the two optimizations we implement here are also relevant for the
hardware accelerated implementation later.*

## Directory Structure
* `Makefile`: Makefile to build, run, and test everything.
* `app/`: Application code we are running for our measurements. Runs inside the
  simulated machine.
  + `app/matrixmultiply.c`: Simple baseline matrix multiplication.
  + `app/matrixmultiply_block.c`: Optimized multiplication you are implementing
    in this milestone.
* `tests/`: configurations for all the tests.
  + `app/test*.sim.py`: Simulation configuration
  + `app/test*.check.py`: Automated check of simulation result
* `out/`: Outputs from completed simulation runs.

## Simulations
We will simulate a machine with a single-core in-order (both to keep simulation
times manageable) X86 processor, with 32KB L1 data and instruction caches, a 2MB
L2 cache, and a 32MB L3 cache. The simulated machine has 512MB DDR4 memory. The
simulated machine will boot a complete Linux kernel and run the program as a
regular application. We run two types of simulation, fast and only functional
(gem5 with KVM CPU virtualization support), and slow and performance-accurate
(gem5 timing).

To keep simulation times low for timing simulation, we configure them to first
boot up the machine with KVM, and then switch over to the slower timing mode
right before actually starting our application. This will show up as two
separate simulation runs per test. Suffix `-0` is for booting up and then
storing a checkpoint  of the system state, while suffix `-1` restores the
checkpoint and then runs with the slower mode.

### Running Simulations
The simulations can be run with `make test`. This will sequentially run all of
them. Expect this to take around one hour. If you have enough cores and
memory you can parallelize them with `make -jN test PARALLEL=y`. `-jN` will
allow make to run up to `N` tests in parallel, while `PARALLEL=y` will allow
test 3 which has multiple runs to run the runs in the test in parallel as well.
With full parallelization the longest test will take 15 minutes.
You can run individual tests with `make testN.out`.

### Orchestration Scripts
The `testN.sim.py` scripts in `test/` specify different simulation
configurations. In this milestone, the only differences are:

1. different applications and commandline parameters to them.
2. different types of simulation modes: functional vs timing.

Please have a look at the orchestration scripts and make sure you understand
what is going on. The
[SimBricks documentation](https://simbricks.readthedocs.io/en/latest/user/orchestration.html)
should have the missing pieces.

### Outputs
Each simulation will output to the command line as it is running. Additionally,
all output is also stored in a json file in `out/testX*.json` (again remember
for timing simulations the interesting things post checkpoint will be in
`out/testX-1.json`). To manually read, you can use
`json_pp <out/testX-1.json | less`. Additionally each simulators might store
additional output files in `out/testX/N/`. For example,
`out/test1/1/gem5-out.host/stats.txt` contains many very detailed performance
statistics for gem5, that can help understand performance of a simulated system.

### Checking Correctness
`make check` will validate all test outputs, if they exist or print failures
otherwise. After running `make test` or running tests individually `make check`
should print success for each test.

## Step 1: Functional Test of Basic Matrix Multiply
As a first step, have a look at the baseline matrix multiplication
implementation in `app/matrixmultiply.c` and make sure you understand it.
Basically the application multiplies two 256x256 matrices ten times and measures
the number of processor cycles required for each multiplication.
Next, you can build it with `make all`.

Now it's time for running your first simulation. `make test0.out` will run the
first functional test. This will generate a fair bit of output, but the
interesting piece will be towards the end. Here the simulated system runs the
application and the application prints out it's (not yet meaningful) cycle
measurement:
```
host.host OUT: ['+ /tmp/guest/matrixmultiply\r']
host.host OUT: ['Cycles per operation: 44737817']
```

## Step 2: Performance Measurement of Basic Matrix Multiply
Next, we simulate the same configuration again, but this time with the more
accurate gem5 timing mode, so we get interpretable performance results. This
will take around 15 minutes, but at the end you should again find the similar
lines:
```
$ make test1.out
...
host.host OUT: ['+ /tmp/guest/matrixmultiply\r']
host.host OUT: ['Cycles per operation: 304543155\r']
```

You can find a lot of internal detail about what the system was up to in the
simulator statistics: `out/test1/1/gem5-out.host/stats.txt`. For example, an
interesting metric is the L1 data cache miss rate
(`system.cpu.dcache.overall_miss_rate::.switch_cpus.data`).

## Step 3: Implement Block Matrix Multiplication
Next or in parallel, you will implement an optimized version of the matrix
multiplication. The idea is to leverage
[block matrix multiplication](https://en.wikipedia.org/wiki/Block_matrix#Block_matrix_multiplication).
Effectively carving up a large matrix multiply into many smaller
matrix multiplies. Please implement this in the indicated location in
`app/matrixmultiply_block.c`. The skeleton already has functionality for testing
this against the simple multiply as well as measuring its performance.

The main piece to implement is this function:
```
void matmult_block(const uint8_t *inA, const uint8_t *inB, uint8_t *out,
    size_t n, size_t block)
```
`inA` and `inB` are the two 8-bit unsigned integer matrices to multiply, both
are of size `n` x `n`. The output matrix is to be stored in `out`. The
multiplication should happen in blocks of `block` x `block`.
*For simplicity we only consider square matrices and square blocks.*

The reference implementation is around 150 lines of code, including skeleton and
license header.

## Step 4: Functional Testing of Improved Matrix Multiply
Once implemented, you can test your code by running `test2`, which will run the
functional comparison to the baseline in a fast KVM simulation and print out
success or failure:
```
$ make test2.out
...
host.host OUT: ['+ /tmp/guest/matrixmultiply_block 512 64\r']
host.host OUT: ['STATUS: Success Matrices match\r']
```

## Step 5: Performance Measurement of Improved Matrix Multiply
After correcting correctness, it is time to measure its performance and compare
it to the baseline for different blocksizes. `tests/test3.py` shows how to do
multiple experiment runs with different parameters. You should see around 40%
fewer cycles per operation:
```
$ make test3.out
...
host.host OUT: ['+ /tmp/guest/matrixmultiply_block 256 64 10\r']
host.host OUT: ['Cycles per operation: 188446607\r']
```

**Speeding up with parallel simulations:** you can speed this up by
parallelizing at the cost of interleaved console output. For this you can
run `make` with `PARALLEL=y` as shown in
[Running Simulations](#running-simulations)

## Step 6: Implement, Test, Measure Block Multiply with Transposition
Finally, you should implement another optimization. Start by thinking through
the memory access pattern for the matrix multiply, and how accesses to the two
input matrices differ in terms of cache locality. This behavior can be
significantly improved by transposing one of the input matrices first (which
one?) and then changing the multiplication loop to use the transposed matrix.
(note that the function inputs are `const`, so don't transpose in-place).

Implement this in `app/matrixmultiply_block_t.c`. The reference implementation
is around 165 lines including skeleton and license header.

Tests 4 and 5 test for the functional correctness and performance for the
optimized multiplication:
```
$ make test4.out
...
host.host OUT: ['+ /tmp/guest/matrixmultiply_block_t 512 64\r']
host.host OUT: ['STATUS: Success Matrices match\r']
...
$ make test5.out
...
host.host OUT: ['+ /tmp/guest/matrixmultiply_block_t 256 64 10\r']
host.host OUT: ['Cycles per operation: 40039540\r']
```
