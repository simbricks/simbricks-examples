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

// these are initialized from the main function based on command line parameters
uint64_t op_latency;
uint64_t matrix_size;


int InitState(void) {
  // FILL ME IN
  return 0;
}

void MMIORead(volatile struct SimbricksProtoPcieH2DRead *read)
{
#ifdef DEBUG
  fprintf(stderr, "MMIO Read: BAR %d offset 0x%lx len %d\n", read->bar,
    read->offset, read->len);
#endif

  // prepare read completion
  volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
  volatile struct SimbricksProtoPcieD2HReadcomp *rc = &msg->readcomp;
  rc->req_id = read->req_id; // set req id so host can match resp to a req

  // zero it out in case of bad register
  memset((void *) rc->data, 0, read->len);

  uint64_t val = 0;
  void *src = NULL;

  // YOU WILL NEED TO CHANGE THIS SUBSTANTIALLY, THIS IS JUST AN EXAMPLE
  if (read->offset < 64) {
    // design choice: All our actual registers need to be accessed with 64-bit
    // aligned reads
    assert(read->len <= 8);
    assert(read->offset % read->len == 0);

    switch (read->offset) {
      case REG_SIZE: val = 42; break;
    }
    src = &val;
  } else {
    fprintf(stderr, "MMIO Read: warning invalid MMIO read 0x%lx\n",
          read->offset);
  }

  // copy data into response message
  if (src)
    memcpy((void *) rc->data, src, read->len);

  // send response
  SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
}

void MMIOWrite(volatile struct SimbricksProtoPcieH2DWrite *write)
{
#ifdef DEBUG
  fprintf(stderr, "MMIO Write: BAR %d offset 0x%lx len %d\n", write->bar,
    write->offset, write->len);
#endif

  // YOU WILL NEED TO CHANGE THIS SUBSTANTIALLY, THIS IS JUST AN EXAMPLE
  if (write->offset < 64) {
    assert(write->len <= 8);
    assert(write->offset % write->len == 0);
    uint64_t val = 0;
    memcpy(&val, (const void *) write->data, write->len);
    switch (write->offset) {
      default:
        fprintf(stderr, "MMIO Write: warning write to invalid register "
                        "0x%lx = 0x%lx\n",
                write->offset, val);
    }
  } else {
    fprintf(stderr, "MMIO Write: warning invalid MMIO write 0x%lx\n",
          write->offset);
  }

  // note that writes need no completion
}

void PollEvent(void) {
  // FILL ME IN
}

uint64_t NextEvent(void) {
  // FILL ME IN
  return UINT64_MAX;
}
