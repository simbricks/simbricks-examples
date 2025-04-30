/*
 * Copyright 2024 Max Planck Institute for Software Systems, and
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
// #define AXI_R_DEBUG
// #define AXI_W_DEBUG

#include <signal.h>
#include <verilated.h>

#include <cassert>

#include "VVTAShell.h"
#include "simbricks/pcie/proto.h"

// #define TRACE_ENABLED

#include <cstdlib>
#include <iostream>
#include <memory>
#ifdef TRACE_ENABLED
#include <verilated_vcd_c.h>
#endif

#include <simbricks/base/cxxatomicfix.h>
extern "C" {
#include <simbricks/nicif/nicif.h>
}

#include "vta_simbricks.hh"

uint64_t clock_period = 10 * 1000ULL;  // 10ns -> 100MHz

volatile int exiting = 0;
bool terminated = false;
uint64_t main_time = 0;
struct SimbricksNicIf nicif {};

std::unique_ptr<VVTAShell> shell{};
std::unique_ptr<VTAAXISubordinateRead> dma_read{};
std::unique_ptr<VTAAXISubordinateWrite> dma_write{};
std::unique_ptr<VTAAXILManager> reg_read_write{};

volatile union SimbricksProtoPcieD2H *d2h_alloc();

void sigint_handler(int dummy) {
  exiting = 1;
}

void sigusr1_handler(int dummy) {
  fprintf(stderr, "main_time = %lu\n", main_time);
}

void VTAAXISubordinateRead::do_read(const simbricks::AXIOperation &axi_op) {
  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc();
  if (!msg) {
    throw "VTAAXISubordinateRead::do_read() SimBricks msg alloc failed";
  }
  volatile struct SimbricksProtoPcieD2HRead &read = msg->read;
  read.req_id = axi_op.id;
  read.offset = axi_op.addr;
  read.len = axi_op.len;

  if (SimbricksPcieIfH2DOutMsgLen(&nicif.pcie) -
          sizeof(SimbricksProtoPcieH2DReadcomp) <
      axi_op.len) {
    throw "VTAAXISubordinateRead::do_read() read response can't fit the the requested data";
  }
  SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_READ);
}

void VTAAXISubordinateWrite::do_write(const simbricks::AXIOperation &axi_op) {
  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc();
  if (!msg) {
    throw "VTAAXISubordinateWrite::do_write() SimBricks msg alloc failed";
  }
  volatile struct SimbricksProtoPcieD2HWrite &write = msg->write;
  write.req_id = axi_op.id;
  write.offset = axi_op.addr;
  write.len = axi_op.len;

  if (SimbricksPcieIfD2HOutMsgLen(&nicif.pcie) -
          sizeof(SimbricksProtoPcieD2HWrite) <
      axi_op.len) {
    throw "VTAAXISubordinateWrite::do_write() write message can't fit the data";
  }

  memcpy(const_cast<uint8_t *>(write.data), axi_op.buf.get(), axi_op.len);
  SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITE);
}

void VTAAXILManager::read_done(simbricks::AXILOperationR &axi_op) {
  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc();
  if (!msg) {
    throw "VTAAXILManager::read_done() completion alloc failed";
  }
  volatile struct SimbricksProtoPcieD2HReadcomp &readcomp = msg->readcomp;
  memcpy(const_cast<uint8_t *>(readcomp.data), &axi_op.data, AXIL_BYTES_DATA);
  readcomp.req_id = axi_op.req_id;
  SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
}

void VTAAXILManager::write_done(simbricks::AXILOperationW &axi_op) {
  if (axi_op.posted) {
    return;
  }
  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc();
  if (!msg) {
    throw "VTAAXILManager::write_done() completion alloc failed";
  }
  volatile struct SimbricksProtoPcieD2HWritecomp &writecomp = msg->writecomp;
  writecomp.req_id = axi_op.req_id;
  SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
}

void h2d_read(volatile struct SimbricksProtoPcieH2DRead &read) {
  if (read.bar == 0) {
    reg_read_write->issue_read(read.req_id, read.offset);
  } else {
    throw "h2d_read() unexpected bar";
  }
}

void h2d_write(volatile struct SimbricksProtoPcieH2DWrite &write,
               bool isPosted) {
  if (write.bar == 0) {
    if (write.len != AXIL_BYTES_DATA) {
      throw "h2d_write() write must be of full width";
    }
    uint64_t data = 0;
    memcpy(&data, const_cast<uint8_t *>(write.data), AXIL_BYTES_DATA);
    reg_read_write->issue_write(write.req_id, write.offset, data, isPosted);
  } else {
    throw "h2d_write() unexpected bar";
  }
}

void h2d_readcomp(volatile struct SimbricksProtoPcieH2DReadcomp &readcomp) {
  dma_read->read_done(readcomp.req_id, const_cast<uint8_t *>(readcomp.data));
}

void h2d_writecomp(volatile struct SimbricksProtoPcieH2DWritecomp &writecomp) {
  dma_write->write_done(writecomp.req_id);
}

void poll_h2d() {
  volatile union SimbricksProtoPcieH2D *msg =
      SimbricksPcieIfH2DInPoll(&nicif.pcie, main_time);
  uint16_t type;

  if (msg == nullptr)
    return;

  type = SimbricksPcieIfH2DInType(&nicif.pcie, msg);
  switch (type) {
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READ:
      h2d_read(msg->read);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE:
      h2d_write(msg->write, false);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE_POSTED:
      h2d_write(msg->write, true);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP:
      h2d_readcomp(msg->readcomp);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP:
      h2d_writecomp(msg->writecomp);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_DEVCTRL:
    case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
      break;

    case SIMBRICKS_PROTO_MSG_TYPE_TERMINATE:
      std::cerr << "poll_h2d: peer terminated\n";
      terminated = true;
      break;

    default:
      std::cerr << "poll_h2d: unsupported type=" << type << "\n";
  }

  SimbricksPcieIfH2DInDone(&nicif.pcie, msg);
}

volatile union SimbricksProtoPcieD2H *d2h_alloc() {
  return SimbricksPcieIfD2HOutAlloc(&nicif.pcie, main_time);
}

int main(int argc, char *argv[]) {
  struct SimbricksBaseIfParams pcie_params;

  SimbricksPcieIfDefaultParams(&pcie_params);

  if (argc < 3 || argc > 7) {
    fprintf(stderr,
            "Usage: vta_simbricks PCI-SOCKET SHM [START-TICK] "
            "[SYNC-PERIOD] [PCI-LATENCY] [CLOCK-FREQ-MHZ]\n");
    return EXIT_FAILURE;
  }
  if (argc >= 4)
    main_time = strtoull(argv[3], NULL, 0);
  if (argc >= 5)
    pcie_params.sync_interval = strtoull(argv[4], NULL, 0) * 1000ULL;
  if (argc >= 6)
    pcie_params.link_latency = strtoull(argv[5], NULL, 0) * 1000ULL;
  if (argc >= 7)
    clock_period = 1000000ULL / strtoull(argv[6], NULL, 0);

  struct SimbricksProtoPcieDevIntro dev_intro;
  memset(&dev_intro, 0, sizeof(dev_intro));

  dev_intro.bars[0].len = 4096;
  dev_intro.bars[0].flags = 0;

  dev_intro.pci_vendor_id = 0xdead;
  dev_intro.pci_device_id = 0xbeef;
  dev_intro.pci_class = 0x40;
  dev_intro.pci_subclass = 0x00;
  dev_intro.pci_revision = 0x00;

  pcie_params.sock_path = argv[1];

  if (SimbricksNicIfInit(&nicif, argv[2], nullptr, &pcie_params, &dev_intro)) {
    return EXIT_FAILURE;
  }
  int sync_pci = SimbricksBaseIfSyncEnabled(&nicif.pcie.base);

  signal(SIGINT, sigint_handler);
  signal(SIGUSR1, sigusr1_handler);

  /* initialize verilated model */
  shell = std::make_unique<VVTAShell>();

#ifdef TRACE_ENABLED
  Verilated::traceEverOn(true);
  VerilatedVcdC trace{};
  shell->trace(&trace, 99);
  trace.open("debug.vcd");
#endif

  reg_read_write = std::make_unique<VTAAXILManager>(*shell);
  dma_read = std::make_unique<VTAAXISubordinateRead>(*shell);
  dma_write = std::make_unique<VTAAXISubordinateWrite>(*shell);

  // reset HW
  for (int i = 0; i < 10; i++) {
    shell->reset = 1;
    shell->clock = 0;
    main_time += clock_period / 2;
    shell->eval();
#ifdef TRACE_ENABLED
    trace.dump(main_time);
#endif
    shell->clock = 1;
    main_time += clock_period / 2;
    shell->eval();
#ifdef TRACE_ENABLED
    trace.dump(main_time);
#endif
  }
  shell->reset = 0;

  /* main simulation loop */
  while (!exiting) {
    int done;
    do {
      done = 1;
      if (SimbricksPcieIfD2HOutSync(&nicif.pcie, main_time) < 0) {
        std::cerr << "warn: SimbricksPcieIfD2HOutSync failed (t=" << main_time
                  << ")" << "\n";
        done = 0;
      }
      if (SimbricksNetIfOutSync(&nicif.net, main_time) < 0) {
        std::cerr << "warn: SimbricksNetIfOutSync failed (t=" << main_time
                  << ")" << "\n";
        done = 0;
      }
    } while (!done);

    do {
      poll_h2d();
    } while (!exiting && ((sync_pci && SimbricksPcieIfH2DInTimestamp(
                                           &nicif.pcie) <= main_time)));

    /* falling edge */
    shell->clock = 0;
    main_time += clock_period / 2;
    shell->eval();
#ifdef TRACE_ENABLED
    trace.dump(main_time);
#endif

    /* raising edge */
    shell->clock = 1;
    main_time += clock_period / 2;
    reg_read_write->step(main_time);
    dma_read->step(main_time);
    dma_write->step(main_time);
    shell->eval();
    reg_read_write->step_apply();
    dma_read->step_apply();
    dma_write->step_apply();
#ifdef TRACE_ENABLED
    trace.dump(main_time);
#endif
  }

#ifdef TRACE_ENABLED
  trace.dump(main_time + 1);
  trace.close();
#endif
  shell->final();
  shell = nullptr;
  return 0;
}
