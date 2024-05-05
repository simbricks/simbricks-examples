/*
 * Copyright 2023 Max Planck Institute for Software Systems, and
 * National University of Singapore
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>

extern "C" {
#include "../accel-sim/sim.h"
#include "../common/reg_defs.h"
}

//#define DEBUG
#ifdef DEBUG
#define dprintf(x...) do { \
    fprintf(stderr, "DEBUG [%lu]: ", main_time); \
    fprintf(stderr, x); \
  } while (0)
#else
#define dprintf(...) do { } while (0)
#endif

#define TRACE_ENABLED
#ifdef TRACE_ENABLED
#include <verilated_vcd_c.h>
#endif
#include <Vtop.h>

// uncomment to enable debug prints
//#define DEBUG

uint64_t clock_period = 10000;

static uint64_t next_edge = 0;
static bool next_rising = false;
static int reset_done = 2;

struct MMIOOp {
  uint64_t offset;
  uint64_t val;
  bool write;
  uint64_t opaque;
};

struct DMAOp {
  uint8_t *data;
  uint64_t addr;
  uint64_t offset;
  uint64_t len;
  uint64_t pos;
  bool write;
  enum {
    DMAST_READY,
    DMAST_TRANSFERING,
    DMAST_ISSUED,
  } state;
};

static std::deque<MMIOOp *> mmio_queue;
static bool mmio_submitted = false;

static DMAOp *dma_op = nullptr;
static bool dma_submitted = false;
static uint64_t dma_len;
static uint64_t dma_addr;
static uint64_t dma_off;

static Vtop *top;

#ifdef TRACE_ENABLED
static VerilatedVcdC *trace;
#endif

int InitState(void) {
  char arg0[] = "hw/hw";
  char *vargs[2] = {arg0, NULL};
  Verilated::commandArgs(1, vargs);
#ifdef TRACE_ENABLED
  Verilated::traceEverOn(true);
#endif
  top = new Vtop;

#ifdef TRACE_ENABLED
  trace = new VerilatedVcdC;
  top->trace(trace, 99);
  trace->open("out/debug.vcd");
#endif

  top->rst = 1;
  return 0;
}

void Finalize(void) {
  trace->close();
}

static bool MMIOReadDMA(volatile struct SimbricksProtoPcieH2DRead *read) {
  uint64_t val = 0;

  switch (read->offset) {
    case REG_DMA_CTRL:
      val = dma_op ? REG_DMA_CTRL_RUN : 0;
      break;
    case REG_DMA_LEN:
      val = dma_len;
      break;
    case REG_DMA_ADDR:
      val = dma_addr;
      break;
    case REG_DMA_OFF:
      val = dma_off;
      break;
    default:
      return false;
  }

  assert(read->len == 8);

  // praepare read completion
  volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
  volatile struct SimbricksProtoPcieD2HReadcomp *rc = &msg->readcomp;
  rc->req_id = read->req_id; // set req id so host can match resp to a req

  memcpy((void *) rc->data, &val, read->len);

  // send response
  SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);

  return true;
}

static bool MMIOWriteDMA(volatile struct SimbricksProtoPcieH2DWrite *write) {
  uint64_t val = 0;

  assert(write->len == 8);
  assert(write->offset % write->len == 0);
  memcpy(&val, (const void *) write->data, write->len);

  switch (write->offset) {
    case REG_DMA_CTRL:
      if ((val & REG_DMA_CTRL_RUN)) {
        if (dma_op) {
          fprintf(stderr, "warn: starting DMA with already running DMA\n");
          return true;
        }

        dma_op = new DMAOp;
        dma_op->data = new uint8_t[dma_len];
        dma_op->offset = dma_off;
        dma_op->addr = dma_addr;
        dma_op->len = dma_len;
        dma_op->write = (val & REG_DMA_CTRL_W);
        dma_op->state = DMAOp::DMAST_READY;
        dma_op->pos = 0;

        dprintf("Received DMA op\n");
      }
      break;
    case REG_DMA_LEN:
      dma_len = val;
      break;
    case REG_DMA_ADDR:
      dma_addr = val;
      break;
    case REG_DMA_OFF:
      dma_off = val;
      break;
    default:
      return false;
  }
  return true;
}

void MMIORead(volatile struct SimbricksProtoPcieH2DRead *read) {
  dprintf("MMIO Read: BAR %d offset 0x%lx len %d id %lu\n", read->bar,
    read->offset, read->len, read->req_id);

  if (MMIOReadDMA(read))
    return;

  assert(read->len == 8);

  MMIOOp *op = new MMIOOp;
  op->offset = read->offset;
  op->write = false;
  op->opaque = read->req_id;
  mmio_queue.push_back(op);
}

void MMIOWrite(volatile struct SimbricksProtoPcieH2DWrite *write) {
  dprintf("MMIO Write: BAR %d offset 0x%lx len %d id %lu\n", write->bar,
    write->offset, write->len, write->req_id);

  if (MMIOWriteDMA(write))
    return;

  assert(write->len == 8);
  assert(write->offset % write->len == 0);

  MMIOOp *op = new MMIOOp;
  op->offset = write->offset;
  op->write = true;
  op->opaque = 0;
  op->opaque = write->req_id;
  memcpy(&op->val, (const void *) write->data, 8);
  mmio_queue.push_back(op);
}

static void MMIOPoll() {
  if (mmio_submitted) {
    // we have an ongoing mmio op that we submitted to the device and is now
    // done
    MMIOOp *op = mmio_queue.front();
    mmio_queue.pop_front();

    // for writes we don't have to do anything else, but for reads we do
    if (op->write) {
      dprintf("MMIO Write Complete: id %lu\n", op->opaque);
    } else {
      dprintf("MMIO Read Complete: id %lu val %lx\n", op->opaque,
        (uint64_t) top->mmio_r_data);
      // praepare read completion
      volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
      volatile struct SimbricksProtoPcieD2HReadcomp *rc = &msg->readcomp;
      rc->req_id = op->opaque; // set req id so host can match resp to a req
      memcpy((void *) rc->data, &top->mmio_r_data, 8);
      SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
    }

    delete op;
    mmio_submitted = false;
  }

  if (!mmio_queue.empty()) {
    // we have a next request to submit
    MMIOOp *op = mmio_queue.front();
    if (op->write) {
      dprintf("MMIO Write Submit: id %lu\n", op->opaque);
      top->mmio_w_req = 1;
      top->mmio_w_addr = op->offset;
      top->mmio_w_data = op->val;
      top->mmio_r_req = 0;
    } else {
      dprintf("MMIO Read Submit: id %lu\n", op->opaque);
      top->mmio_w_req = 0;
      top->mmio_r_req = 1;
      top->mmio_r_addr = op->offset;
    }

    mmio_submitted = true;
  } else {
    top->mmio_w_req = 0;
    top->mmio_r_req = 0;
  }
}

static void DMAPoll() {
  if (!dma_op)
    return;

  top->dma_w_req = 0;
  top->dma_r_req = 0;
  if (dma_op->write) {
    if (dma_op->state == DMAOp::DMAST_READY) {
      dprintf("starting DMA Write op\n");
      top->dma_r_req = 1;
      top->dma_r_addr = dma_op->offset;
      dma_op->state = DMAOp::DMAST_TRANSFERING;
      dma_op->pos = 0;
    } else if (dma_op->state == DMAOp::DMAST_TRANSFERING) {
      dprintf("starting Stepping DMA Write\n");
      memcpy(dma_op->data + dma_op->pos, &top->dma_r_data,
        sizeof(top->dma_r_data));
      dma_op->pos += sizeof(top->dma_r_data);

      if (dma_op->pos >= dma_op->len) {
        dprintf("Write transfer done, issuing\n");
        IssueDMAWrite(dma_op->addr, dma_op->data, dma_op->len, 0);
        dma_op->state = DMAOp::DMAST_ISSUED;
      } else {
        dprintf("starting continuing DMA read\n");
        top->dma_r_req = 1;
        top->dma_r_addr = dma_op->offset + dma_op->pos;
      }
    }
  } else {
    if (dma_op->state == DMAOp::DMAST_READY) {
      dprintf("starting Issuning Read op: A=%lx O=%lx L=%lu \n",
        dma_op->addr, dma_op->offset, dma_op->len);
      IssueDMARead(dma_op->data, dma_op->addr, dma_op->len, 0);
      dma_op->state = DMAOp::DMAST_ISSUED;
    } else if (dma_op->state == DMAOp::DMAST_TRANSFERING) {
      dprintf("Stepping Read op: pos=%lu\n", dma_op->pos);
      if (dma_op->pos >= dma_op->len) {
        dprintf("Read done\n");
        delete[] dma_op->data;
        delete dma_op;
        dma_op = nullptr;
      } else {
        dprintf("Read continuing\n");
        top->dma_w_req = 1;
        top->dma_w_addr = dma_op->offset + dma_op->pos;
        memcpy(&top->dma_w_data, dma_op->data + dma_op->pos,
          sizeof(top->dma_w_data));
        dma_op->pos += sizeof(top->dma_w_data);
      }
    }
  }
}

void PollEvent(void) {
  if (main_time < next_edge)
    return;

  if (next_rising) {
    MMIOPoll();
    DMAPoll();
  }

  // here we just jiggle the clock
  top->clk = next_rising;
  next_rising = !next_rising;

  top->eval();
#ifdef TRACE_ENABLED
  trace->dump(main_time);
#endif

  if (!reset_done && !next_rising) {
    top->rst = 0;
  } else if (next_rising) {
    reset_done--;
  }
  next_edge += clock_period / 2;
}

uint64_t NextEvent(void) {
  return next_edge;
}

void DMACompleteEvent(uint64_t opaque) {
  dprintf("DMA Completed\n");
  if (dma_op->write) {
    delete dma_op->data;
    delete dma_op;
    dma_op = nullptr;
  } else {
    dma_op->state = DMAOp::DMAST_TRANSFERING;
  }
}