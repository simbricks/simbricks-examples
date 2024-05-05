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

#include "plumbing.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simbricks/pcie/if.h>
#include "sim.h"

static struct SimbricksBaseIfParams pcie_params;
static const char *shm_pool_path;

struct SimbricksPcieIf pcie_if;
static bool is_sync;
uint64_t main_time = 0;
static uint64_t max_step = 10000;
static volatile bool exiting = false;

static uint64_t mmio_reads = 0;
static uint64_t mmio_writes = 0;
static uint64_t dma_reads = 0;
static uint64_t dma_writes = 0;

static void PollPcie(void);

static int ParseOptions(int argc, char *argv[]) {
    SimbricksPcieIfDefaultParams(&pcie_params);

    if (argc < 7 && argc > 13) {
        fprintf(stderr,
                "Usage: accel-sim OP-LATENCY MATRIX-SIZE MEM-SIZE PCI-SOCKET "
                "SHM  [START-TICK] [SYNC-PERIOD] [PCI-LATENCY]\n");
        return EXIT_FAILURE;
    }

    op_latency = strtoull(argv[1], NULL, 0);
    matrix_size = strtoull(argv[2], NULL, 0);
    mem_size = strtoull(argv[3], NULL, 0);

    pcie_params.sock_path = argv[4];
    shm_pool_path = argv[5];
    if (argc >= 7)
        main_time = strtoull(argv[6], NULL, 0);
    if (argc >= 8)
        pcie_params.sync_interval = strtoull(argv[7], NULL, 0) * 1000ULL;
    if (argc >= 9)
        pcie_params.link_latency = strtoull(argv[8], NULL, 0) * 1000ULL;

    return 0;
}

static int InitSimBricks(void) {
    struct SimbricksBaseIfSHMPool pool;

    struct SimbricksProtoPcieDevIntro pcie_d_intro;
    memset(&pcie_d_intro, 0, sizeof(pcie_d_intro));

    pcie_d_intro.bars[0].len = 1 << 24;
    pcie_d_intro.bars[0].flags = SIMBRICKS_PROTO_PCIE_BAR_64;

    pcie_d_intro.pci_vendor_id = 0x9876;
    pcie_d_intro.pci_device_id = 0x1234;

    pcie_d_intro.pci_class = 0x40;
    pcie_d_intro.pci_subclass = 0x00;
    pcie_d_intro.pci_revision = 0x00;
    pcie_d_intro.pci_msi_nvecs = 32;

    size_t shm_size = SimbricksBaseIfSHMSize(&pcie_params);
    if (SimbricksBaseIfSHMPoolCreate(&pool, shm_pool_path, shm_size)) {
        perror("SimbricksBaseIfSHMPoolCreate failed");
        return -1;
    }

    if (SimbricksBaseIfInit(&pcie_if.base, &pcie_params)) {
        perror("SimbricksBaseIfInit failed");
        return -1;
    }


    if (SimbricksBaseIfListen(&pcie_if.base, &pool)) {
      perror("SimbricksBaseIfListen failed");
      return -1;
    }

    struct SimbricksProtoPcieHostIntro pcie_h_intro;

    struct SimBricksBaseIfEstablishData estd;
    estd.base_if = &pcie_if.base;
    estd.tx_intro = &pcie_d_intro;
    estd.tx_intro_len = sizeof(pcie_d_intro);
    estd.rx_intro = &pcie_h_intro;
    estd.rx_intro_len = sizeof(pcie_h_intro);

    if (SimBricksBaseIfEstablish(&estd, 1)) {
        perror("SimBricksBaseIfEstablish failed");
      return -1;
    }

    is_sync = SimbricksBaseIfSyncEnabled(&pcie_if.base);
    return 0;
}

static void RunLoop(void) {
  uint64_t next_ts, next_sync, next_in, next_ev;

  while (!exiting) {
    while (SimbricksPcieIfD2HOutSync(&pcie_if, main_time))
      ;//fprintf(stderr, "warn: Sync failed (t=%lu)\n", main_time);

    do {
      PollPcie();
      PollEvent();

      if (is_sync) {
        next_in = SimbricksPcieIfH2DInTimestamp(&pcie_if);
        next_sync = SimbricksPcieIfD2HOutNextSync(&pcie_if);
        next_ts = (next_in <= next_sync ? next_in : next_sync);
        if (next_ts > main_time + max_step)
          next_ts = main_time + max_step;
      } else {
        next_ts = main_time + max_step;
      }

      next_ev = NextEvent();
      if (next_ev < next_ts)
        next_ts = next_ev;
    } while (next_ts <= main_time && !exiting);

    main_time = next_ts;
  }
}

static void PollPcie(void) {
  volatile union SimbricksProtoPcieH2D *msg =
      SimbricksPcieIfH2DInPoll(&pcie_if, main_time);
  uint8_t type;

  if (msg == NULL)
    return;

  type = SimbricksPcieIfH2DInType(&pcie_if, msg);
  switch (type) {
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READ:
      mmio_reads++;
      MMIORead(&msg->read);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE: {
      mmio_writes++;
      MMIOWrite(&msg->write);

      volatile union SimbricksProtoPcieD2H *omsg = AllocPcieOut();
      volatile struct SimbricksProtoPcieD2HWritecomp *wc = &omsg->writecomp;
      wc->req_id = msg->write.req_id;
      SimbricksPcieIfD2HOutSend(&pcie_if, omsg,
                                SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
      break;
    }

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE_POSTED:
      mmio_writes++;
      MMIOWrite(&msg->write);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP:
      dma_reads++;
      DMAEvent(msg);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP:
      dma_writes++;
      DMAEvent(msg);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_DEVCTRL:
      break;

    case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
      break;

    case SIMBRICKS_PROTO_MSG_TYPE_TERMINATE:
      fprintf(stderr, "poll_h2d: peer terminated\n");
      exiting = true;
      break;

    default:
      fprintf(stderr, "poll_h2d: unsupported type=%u\n", type);
  }

  SimbricksPcieIfH2DInDone(&pcie_if, msg);
}

volatile union SimbricksProtoPcieD2H *AllocPcieOut(void) {
  if (SimbricksBaseIfInTerminated(&pcie_if.base)) {
    fprintf(stderr, "AllocPcieOut: peer already terminated\n");
    abort();
  }

  volatile union SimbricksProtoPcieD2H *msg;
  bool first = true;
  while ((msg = SimbricksPcieIfD2HOutAlloc(&pcie_if, main_time)) == NULL) {
    if (first) {
      fprintf(stderr, "AllocPcieOut: warning waiting for entry (%zu)\n",
              pcie_if.base.out_pos);
      first = false;
    }
  }

  if (!first)
    fprintf(stderr, "AllocPcieOut: entry successfully allocated\n");

  return msg;
}

void SendPcieOut(volatile union SimbricksProtoPcieD2H *msg, uint64_t type)
{
  SimbricksPcieIfD2HOutSend(&pcie_if, msg, type);
}

int main(int argc, char *argv[]) 
{
    if (ParseOptions(argc, argv))
      return 1;

    if (InitState())
      return 1;

    if (InitSimBricks())
      return 1;

    RunLoop();

    fprintf(stderr, "MMIO READS = %lu\n", mmio_reads);
    fprintf(stderr, "MMIO WRITES = %lu\n", mmio_writes);
    fprintf(stderr, "DMA READS = %lu\n", dma_reads);
    fprintf(stderr, "DMA WRITES = %lu\n", dma_writes);

    return 0;
}