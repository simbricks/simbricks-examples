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
#include "../common/reg_defs.h"

// uncomment to enable debug prints
//#define DEBUG
#ifdef DEBUG
#define dprintf(x...) do { \
    fprintf(stderr, "DEBUG [%lu]: ", main_time); \
    fprintf(stderr, x); \
  } while (0)
#else
#define dprintf(...) do { } while (0)
#endif

/* Configuration parameters */
uint64_t op_latency;
uint64_t matrix_size;
uint64_t mem_size;

int InitState(void) {
  // FILL ME IN
  return 0;
}

void MMIORead(volatile struct SimbricksProtoPcieH2DRead *read) {
  dprintf("MMIO Read: BAR %d offset 0x%lx len %d\n", read->bar,
    read->offset, read->len);

  // praepare read completion
  volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
  volatile struct SimbricksProtoPcieD2HReadcomp *rc = &msg->readcomp;
  rc->req_id = read->req_id; // set req id so host can match resp to a req

  // zero it out in case of bad register
  memset((void *) rc->data, 0, read->len);

  uint64_t val = 0;
  void *src = NULL;

  assert(read->len <= 8);
  assert(read->offset % read->len == 0);
  switch (read->offset) {
    case REG_SIZE: val = 42; break;
  
    // FILL ME IN

    default:
      fprintf(stderr, "MMIO Read: warning read from invalid register 0x%lx\n",
        read->offset);
  }
  dprintf("MMIO Read Result: %lx\n", val);
  src = &val;

  if (src)
    memcpy((void *) rc->data, src, read->len);

  // send response
  SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
}

void MMIOWrite(volatile struct SimbricksProtoPcieH2DWrite *write) {
  assert(write->len <= 8);
  assert(write->offset % write->len == 0);
  uint64_t val = 0;
  memcpy(&val, (const void *) write->data, write->len);
  dprintf("MMIO Write: BAR %d offset 0x%lx len %d val %lx\n", write->bar,
    write->offset, write->len, val);
  switch (write->offset) {
    // FILL ME IN
    default:
      fprintf(stderr, "MMIO Write: warning write to invalid register 0x%lx\n",
        write->offset);
  }
}

void PollEvent(void) {
  // FILL ME IN
}

uint64_t NextEvent(void) {
  // FILL ME IN
  return UINT64_MAX;
}

void DMACompleteEvent(uint64_t opaque) {
  // FILL ME IN
}