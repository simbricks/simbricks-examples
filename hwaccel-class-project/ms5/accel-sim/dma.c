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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simbricks/pcie/if.h>
#include "sim.h"
#include "plumbing.h"

/**
 * This file implements the DMA state machine. Two key constraints:
 * 1) no individual DMA op must be larger than 4KB, i.e. we have to split larger
 * transfers.
 * 2) No more than 16 pending DMA operations at a time (approximates PCIe flow
 * control).
 */

#define MAX_DMA_OP_SIZE 4096
#define MAX_DMA_OP_PENDING 16

struct DMAOp {
  uint64_t addr;
  void *ptr;
  uint64_t len;
  uint64_t issue_offset;
  uint64_t opaque;
  uint64_t n_pending;
  struct DMAOp *next;
  bool write;
};

struct DMASubOp {
  struct DMAOp *op;
  uint64_t offset;
  uint16_t len;
};

static size_t n_pending = 0;
static struct DMAOp *pending_dma_first = NULL;
static struct DMAOp *pending_dma_last = NULL;

static void IssuePending () {
  struct DMAOp *op;
  for (op = pending_dma_first; op && n_pending < MAX_DMA_OP_PENDING ;) {
    if (op->issue_offset >= op->len) {
      op = op->next;
      continue;
    }

    size_t len = op->len - op->issue_offset;
    if (len > MAX_DMA_OP_SIZE)
      len = MAX_DMA_OP_SIZE;

    struct DMASubOp *sop = malloc(sizeof(*sop));
    sop->op = op;
    sop->offset = op->issue_offset;
    sop->len = len;

    volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
    if (op->write) {
      volatile struct SimbricksProtoPcieD2HWrite *w = &msg->write;
      w->req_id = (uintptr_t) sop;
      w->offset = op->addr + op->issue_offset;
      w->len = len;
      memcpy((void *) w->data, ((uint8_t *) (op->ptr)) + op->issue_offset, len);

      SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITE);
    } else {
      volatile struct SimbricksProtoPcieD2HRead *r = &msg->read;
      r->req_id = (uintptr_t) sop;
      r->offset = op->addr + op->issue_offset;
      r->len = len;

      SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READ);
    }

    op->n_pending++;
    op->issue_offset += len;
    n_pending++;
  }
}

static void IssueDMA(struct DMAOp *op) {
  if (!pending_dma_last) {
    pending_dma_first = op;
    pending_dma_last = op;
  } else {
    pending_dma_last->next = op;
    pending_dma_last = op;
  }

  IssuePending();
}

void IssueDMARead(void *dst, uint64_t src_addr, size_t len, uint64_t opaque) {
  struct DMAOp *op = calloc(1, sizeof(*op));
  op->addr = src_addr;
  op->ptr = dst;
  op->len = len;
  op->opaque = opaque;
  op->write = false;
  IssueDMA(op);
}

void IssueDMAWrite(uint64_t dst_addr, const void *src, size_t len,
    uint64_t opaque) {
  struct DMAOp *op = calloc(1, sizeof(*op));
  op->addr = dst_addr;
  op->ptr = (void *) src;
  op->len = len;
  op->opaque = opaque;
  op->write = true;
  IssueDMA(op);
}

void DMAEvent(volatile union SimbricksProtoPcieH2D *msg) {
  struct DMAOp *op;
  uint8_t type = SimbricksPcieIfH2DInType(&pcie_if, msg);
  if (type == SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP) {
    // Complete a read, involves also copying the data
    struct DMASubOp *sop = (struct DMASubOp *) msg->readcomp.req_id;
    op = sop->op;
    memcpy((uint8_t *) op->ptr + sop->offset, (void *) msg->readcomp.data,
      sop->len);
    free(sop);
  } else if (type == SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP) {
    // for a write this is just the confirmation that it is done
    struct DMASubOp *sop = (struct DMASubOp *) msg->writecomp.req_id;
    op = sop->op;
    free(sop);
  } else {
    fprintf(stderr, "DMAEvent: unexpected event (%u)\n", type);
    abort();
  }

  assert(n_pending > 0);
  n_pending--;
  assert(op->n_pending > 0);
  op->n_pending--;
  if (op->n_pending == 0 && op->issue_offset == op->len) {
    // Operation is now done, yay!
    assert(pending_dma_first == op); // rely on operations completing in order
    pending_dma_first = op->next;
    if (pending_dma_first == NULL)
      pending_dma_last = NULL;

    uint64_t opaque = op->opaque;
    free(op);

    DMACompleteEvent(opaque);
  }

  IssuePending();
}
