# Milestone 4: Asynchronous Hardware-Software Interface
In this fourth milestone, we will further improve the performance of the
hardware-software interface. The key idea is to submit a sequence of
requests/commands to the hardware asynchronously, and on completion of the
sequence let software know. This avoids accelerator resources idling while the
CPU prepares the next command and potentially allows the CPU to do other things
while the accelerator works.


## Directory Structure
* `Makefile`: Makefile to build, run, and test everything.
* `app/`: Application code we are running for our measurements. Runs inside the
  simulated machine.
  + `app/matmul-accel.c`: Simple matrix multiplication test application, uses
    the driver below to issue operations to the accelerator.
    *No need to modify this in ms4.*
  + `app/driver.c`: User-space driver for the hardware accelerator.
* `common/reg_defs.h`: Register and queue definitions for the accelerator
   interface. Used by simulation model and driver.
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
run all of them. Expect this to take around 15 minutes. If you have enough cores
and memory you can parallelize them with `make test PARALLEL=y`. Note that this
will already use 8 cores, so we would suggest not using `-jN` on top of this,
unless you have at least 10 cores. With full parallelization the longest test
will take around 3 minutes. You can run individual tests with `make testN.out`.

`make check` will validate all test outputs, if they exist or print failures
otherwise. After running `make test` or running tests individually `make check`
should print success for each test.


## Efficient, Asynchronous HW-SW Queues
Asynchronous queues between device and software are how most modern devices
communicate efficiently. For efficiency, each of these queues is typically
unidirectional and single-producer/single-consumer, i.e. either goes from
software to device or vice-versa and is only accessed by one thread in software.
The typical implementation is a circular queue of fixed sized elements. This
enables efficient implementation without additional synchronization. In some
devices (e.g. network cards) they are also referred to as descriptor queues,
where each entry is a descriptor for some operation (e.g. a packet received or
to be sent).

HW-SW queues are implemented using the mechanisms we have seen so far. They
reside in physical host memory and can thus be accessed directly by software,
and the device can access them via DMA. In terms of functionality, roles are
largely symmetric, both software and device can directly read and write entries.
However, one key difference arises when it comes to polling: for the CPU
polling a queue is comparatively cheap (for fast devices it is typically the
most efficient option), while for the device fetching an entry to see if it
changed involves a comparatively expensive (latency and energy) DMA read.

As a result, most devices use explicit signalling from software to the device
through MMIO to tell the device how many entries were added. This is typically
done by the driver writing to a `tail index register` that tells the device the
position of the last valid/first invalid element. After this, the device knows
how many entries are ready for processing and can asynchronously go retrieve
them at its own pace. Note that consecutive entries in the queue are contiguous
in physical memory (modulo wrap arounds at the end of the queue), which allows
the device to amortize overheads by fetching a few entries with one DMA read
(typically 4-64) and cache them on the device for subsequent processing. The
driver can also aggregate writes for the tail index register write if enqueuing
multiple operations, by only doing the write once at the end.

To enable bi-directional communication, queues are typically paired up, one in
each direction. For example, the driver might add entries to a request queue,
and the device can then asynchronously send back results or errors through a
response queue, often just linked by a request ID reflected back in the
response. Depending on the device, responses might arrive out of order as well,
especially for more complex devices, e.g. with multiple processing engines.

All queues are allocated by the driver (note that because of DMA this has to be
physically contiguous memory again), which then writes the physical memory
address into a queue base register, and the length into a queue length register.
After this, the driver can add requests to the queue and signal with writes to
the tail index register, and the device writes responses to the response queue,
all without touching base and length registers again.


## Block Matrix Multiply with Asynchronous Queues
Your task in this milestone will be to design and implement such an asynchronous
queue interface for our matrix multiplication accelerator. At the highest level,
the driver should be able to submit a stream of operations for a complete larger
matrix multiplication for the accelerator to complete asynchronously until
finally signalling completion of the last operation.

One way of conceptually thinking about this, is that the queues provide the same
information to the device that the driver has provided synchronously through
MMIO register accesses in `ms3`. The accelerator will thus probably have 3-4
operation types, depending on your implementation: `DMA read`, (maybe `zero
output matrix`, but could do this as a flag to `compute`), `compute`, `DMA
write`. The response queue could be generic and just include one `request
complete` entry type, that just indicates the id and type of command that
completed. Note that not all requests need a response, in fact the driver really
only needs to know when the last request in a sequence for a full multiplication
is complete. We suggest implementing this similar to other devices, by adding a
`request notification` flag to your requests, that the driver then sets only on
the last command.

We provide a suggested set of definitions for the queue registers and in-memory
format in `common/reg_defs.h`. You can choose to use it as is (as in the
reference solution), or modify it as you would like. In addition to the usual
register definitions, this also includes struct/union definitions for queue
entries to make manipulation in driver and simulator easy.

The driver logic for then executing a larger matrix multiplication is similar to
before, in that the driver issues commands for reading matrices into accelerator
buffers, executing block multiplications, and writing back outputs. With the
main difference that this is now fully asynchronous, where the driver prepares
this full sequence of commands, submits it to the accelerator, and then waits
for completion at the end.


## Implementation Suggestions
First off, we will use the same assumptions as at the end of MS3: Accelerator
size is square and a power of two and the accelerator memory is large enough to
fit one row of blocks plus two additional blocks. In this milestone, you can
skip logic for handling accelerators with less memory, if you wish. You can also
drop support for the original MMIO-only interface.

Please remember to use `volatile` annotations judiciously when writing memory
that will be read via DMA or reading memory that will be written via DMA,
especially when polling. Otherwise the compiler might break your code through
optimizations that assume no one else is accessing the memory, i.e. skipping
repeated reads without writes in between.

We suggest starting from the state after the final step in `ms3`.

The reference solution comprises a total 295 lines in `sim.c` and 267 lines in
`driver.c` (including license headers, comments, blank lines, etc.).

### State Machine

The device logic here starts to get complicated with many different asynchronous
operations. With the callback based interface it will be easy to write horrible
spaghetti code that is hard to debug. My suggestion to avoid this is to
implement the logic as an explicit and **sequential** state machine, and first
carefully think through the states and what events will cause transitions to
which state.

For example, the reference implementation uses the following states:
```
enum state {
  ST_IDLE, /* no active opreation */
  ST_FETCHING, /* waiting for DMA read completion for request queue entries */
  ST_PROCESSING,  /* ready to execute next request */
  ST_DMA_PENDING, /* waiting for DMA transfer command to complete */
  ST_COMPUTING,   /* waiting compute opreation to complete */
};
static enum state state = ST_IDLE;
```

Once you have designed the state machine (I would actually draw out the diagram,
label the arrows etc. for yourself), you can then add corresponding logic to
your callbacks in `PollEvent` and `DMACompleteEvent` (don't forget `NextEvent`)
that explicitly does one step in the state machine based on the current state.

Note that for this milestone it suffices for the device to do operations
sequentially one at a time. You can optimize this further by doing independent
operations in parallel, e.g. fetch more requests before the prior ones have
completed, but this gets very complicated quickly. So I would advise against
doing this, unless you already have everything working and just want to make it
faster. A simpler optimization is to fetch multiple requests with one DMA and
then process them. This is not quite as fast but a lot simpler.

Note that handling multiple requests in parallel to overlap compute and DMA
could speed things up even more, but that will require more complicated request
queues where the driver can specify dependencies between operations. This is fun
and very effective, but gets very complicated and is well beyond what you need
for `ms4`.

### Debug Prints

Note that at the latest with this milestone, the flow of events in the system is
getting pretty complicated. You are unlikely to get this right on the first try.
The easiest way to debug what's going on etc. is to include detailed debug
prints in your code, especially in the simulator. We suggest adding these step
by step as you develop rather than trying to add them later when you have a bug.
The `sim.c` has `dprintf()` macro that will turn into a no-op if `DEBUG` is not
defined, and also automatically prints simulator timestamps for each event. If
you add in these prints systematically it will be much easier to tell where and
when something goes wrong.


### Suggested Sequence of Steps

Here is one suggested way to go about tackling all of this.

1. During initialization, the driver will need to allocate **physically
  contiguous** memory for the request and response queue, and pass these
  addresses to the device. You can allocate with additional calls to
  `dma_alloc_alloc()`. Also implement corresponding MMIO registers in the
  simulator.

2. To enable asynchronous fetching and write-back, your driver will need to
  place the full matrices in DMA accessible buffers. And to make it easy for the
  device to fetch and write back blocks with DMA, I would suggest using a
  block-based format for these buffers, where you rearrange inputs so the blocks
  are contiguous in memory, and same with the output buffer. Convert inputs when
  copying them in at the start, convert outputs when copying out after the
  accelerator has finished.

3. Next you could implement a synchronous version of the driver that issues one
  command at a time. This will look very similar to `ms3` except that you use
  the queue instead of MMIOs. Start with just a sleep as your mechanism for
  "waiting" for completion (e.g. use `usleep()` from `unistd.h`). This will
  allow you to first focus on the request queue.

5. Implement your request queue logic in the simulator. Make sure that the
  sequence of operations in the device matches what the driver submits. Again,
  make sure you add ample dprintfs here.

6. Once this works, implement driver logic for generating and enqueuing the
  required sequence of operations for executing a larger multiplication. If you
  write separate functions for issuing the different operation types, your loop
  for this can basically look almost the same as before, with the difference
  that operations are not actually done in each iteration, only enqueued. Now
  use a sleep at the end to wait.

7. Implement the response queue logic in the device and driver, and replace the
  sleep. (**Note** that the sleep will not be enough to pass the performance
  tests. ;-) ). For this the driver should indicate to the device which
  request(s) it expects a completion for (only the final DMA write really), and
  the device should add responses only for those requests.


## Testing

This time we only have two tests for the whole thing. One functional test
(`test0`) that just checks if the result of your hardware accelerated (block)
multiplication matches the simple software implementation. Note that the
reference implementation just needs 2 MMIO Reads (matrix & memory size), 5
writes (Req Q base, req Q len, resp Q base, resp Q len, and then finally req Q
tail index):

```
$ make test0.out
...
dev.host.accel ERR: ['MMIO READS = 2']
dev.host.accel ERR: ['MMIO WRITES = 5']
dev.host.accel ERR: ['DMA READS = 330']
dev.host.accel ERR: ['DMA WRITES = 65']
...
host.host OUT: ['STATUS: Success matrices match\r'] 
...

...
TEST test0
SUCCESS test0
```

 `test1` will then test performance which should be around 5%/35%/35% faster
 compared to `ms3` for delays of 1ms/1μs/1ns:

```
$ make test1.out
...
TEST test1
Delay 10000000000 -> 648539688 Cycles/op
Delay 1000000000 -> 72510959 Cycles/op
Delay 1000000 -> 8574785 Cycles/op
Delay 1000 -> 8511392 Cycles/op
SUCCESS test1
```
