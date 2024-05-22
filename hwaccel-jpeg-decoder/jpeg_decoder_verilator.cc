
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
#define AXI_R_DEBUG 0
#define AXI_W_DEBUG 0
#define JPGD_DEBUG 0

#include "include/jpeg_decoder_verilator.hh"

#include <signal.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>

#include "include/jpeg_decoder_regs.hh"
#include <simbricks/base/cxxatomicfix.h>
#include <simbricks/axi/axil_manager.hh>
#include "verilated.h"
extern "C" {
#include <simbricks/pcie/if.h>
}

#include "simbricks/pcie/proto.h"

std::unique_ptr<Vjpeg_decoder> top{};
std::unique_ptr<VerilatedVcdC> trace{};

std::unique_ptr<JpegDecAXISubordinateRead> dma_read{};
std::unique_ptr<JpegDecAXISubordinateWrite> dma_write{};
std::unique_ptr<JpegDecAXILManager> reg_read_write{};

uint64_t clock_period = 1'000'000 / 150ULL;  // 150 MHz
int exiting = 0;
bool tracing_active = false;
std::string trace_filename{};
uint64_t trace_nr = 0;
struct SimbricksPcieIf pcieif;
uint64_t cur_ts = 0;

// Expose control over Verilator simulation through BAR. This represents the
// underlying memory that's being accessed.
VerilatorRegs verilator_regs{};

void JpegDecAXISubordinateRead::do_read(const simbricks::AXIOperation &axi_op) {
#if JPGD_DEBUG
  std::cout << "JpegDecoderMemReader::doRead() ts=" << cur_ts
            << " id=" << axi_op.id << " addr=" << axi_op.addr
            << " len=" << axi_op.len << "\n";
#endif

  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
  if (!msg) {
    throw "JpegDecAXISubordinateRead::doRead() dma read alloc failed";
  }

  unsigned int max_size = SimbricksPcieIfH2DOutMsgLen(&pcieif) -
                          sizeof(SimbricksProtoPcieH2DReadcomp);
  if (axi_op.len > max_size) {
    std::cerr << "error: read data of length " << axi_op.len
              << " doesn't fit into a SimBricks message\n";
    std::terminate();
  }

  volatile struct SimbricksProtoPcieD2HRead *read = &msg->read;
  read->req_id = axi_op.id;
  read->offset = axi_op.addr;
  read->len = axi_op.len;
  SimbricksPcieIfD2HOutSend(&pcieif, msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READ);
}

void JpegDecAXISubordinateWrite::do_write(
    const simbricks::AXIOperation &axi_op) {
#if JPGD_DEBUG
  std::cout << "JpegDecoderMemWriter::doWrite() ts=" << cur_ts
            << " id=" << axi_op.id << " addr=" << axi_op.addr
            << " len=" << axi_op.len << "\n";
#endif

  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
  if (!msg) {
    throw "JpegDecoderMemWriter::doWrite() dma read alloc failed";
  }

  volatile struct SimbricksProtoPcieD2HWrite *write = &msg->write;
  unsigned int max_size = SimbricksPcieIfH2DOutMsgLen(&pcieif) - sizeof(*write);
  if (axi_op.len > max_size) {
    std::cerr << "error: write data of length " << axi_op.len
              << " doesn't fit into a SimBricks message\n";
    std::terminate();
  }

  write->req_id = axi_op.id;
  write->offset = axi_op.addr;
  write->len = axi_op.len;
  std::memcpy(const_cast<uint8_t *>(write->data), axi_op.buf.get(), axi_op.len);
  SimbricksPcieIfD2HOutSend(&pcieif, msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITE);
}

void JpegDecAXILManager::read_done(simbricks::AXILOperationR &axi_op) {
#if JPGD_DEBUG
  std::cout << "JpegDecAXILManager::read_done() ts=" << cur_ts
            << " id=" << axi_op.req_id << " addr=" << axi_op.addr << "\n";
#endif

  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
  if (!msg) {
    throw "JpegDecAXILManager::read_done() completion alloc failed";
  }

  volatile struct SimbricksProtoPcieD2HReadcomp *readcomp = &msg->readcomp;
  std::memcpy(const_cast<uint8_t *>(readcomp->data), &axi_op.data, 4);
  readcomp->req_id = axi_op.req_id;
  SimbricksPcieIfD2HOutSend(&pcieif, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
}

void JpegDecAXILManager::write_done(simbricks::AXILOperationW &axi_op) {
#if JPGD_DEBUG
  std::cout << "JpegDecAXILManager::write_done ts=" << cur_ts
            << " id=" << axi_op.req_id << " addr=" << axi_op.addr << "\n";
#endif

  if (axi_op.posted) {
    return;
  }

  volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
  if (!msg) {
    throw "JpegDecAXILManager::write_done completion alloc failed";
  }

  volatile struct SimbricksProtoPcieD2HWritecomp *writecomp = &msg->writecomp;
  writecomp->req_id = axi_op.req_id;
  SimbricksPcieIfD2HOutSend(&pcieif, msg,
                            SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
}

extern "C" void sigint_handler(int dummy) {
  exiting = 1;
}

extern "C" void sigusr1_handler(int dummy) {
  std::cerr << "cur_ts=" << cur_ts << "\n";
}

volatile union SimbricksProtoPcieD2H *d2h_alloc(uint64_t cur_ts) {
  volatile union SimbricksProtoPcieD2H *msg;
  while (!(msg = SimbricksPcieIfD2HOutAlloc(&pcieif, cur_ts))) {
  }
  return msg;
}

bool PciIfInit(const char *shm_path,
               struct SimbricksBaseIfParams &baseif_params) {
  struct SimbricksBaseIfSHMPool pool;
  struct SimBricksBaseIfEstablishData ests;
  struct SimbricksProtoPcieDevIntro d_intro;
  struct SimbricksProtoPcieHostIntro h_intro;

  std::memset(&pool, 0, sizeof(pool));
  std::memset(&ests, 0, sizeof(ests));
  std::memset(&d_intro, 0, sizeof(d_intro));

  d_intro.pci_vendor_id = 0xdead;
  d_intro.pci_device_id = 0xbeef;
  d_intro.pci_class = 0x40;
  d_intro.pci_subclass = 0x00;
  d_intro.pci_revision = 0x00;

  static_assert(sizeof(VerilatorRegs) <= 4096, "Registers don't fit BAR");
  d_intro.bars[0].len = 4096;
  d_intro.bars[0].flags = 0;
  static_assert(sizeof(JpegDecoderRegs) <= 4096, "Registers don't fit BAR");
  d_intro.bars[1].len = 4096;
  d_intro.bars[1].flags = 0;

  ests.base_if = &pcieif.base;
  ests.tx_intro = &d_intro;
  ests.tx_intro_len = sizeof(d_intro);
  ests.rx_intro = &h_intro;
  ests.rx_intro_len = sizeof(h_intro);

  if (SimbricksBaseIfInit(&pcieif.base, &baseif_params)) {
    std::cerr << "PciIfInit: SimbricksBaseIfInit failed\n";
    return false;
  }

  if (SimbricksBaseIfSHMPoolCreate(
          &pool, shm_path, SimbricksBaseIfSHMSize(&pcieif.base.params)) != 0) {
    std::cerr << "PciIfInit: SimbricksBaseIfSHMPoolCreate failed\n";
    return false;
  }

  if (SimbricksBaseIfListen(&pcieif.base, &pool) != 0) {
    std::cerr << "PciIfInit: SimbricksBaseIfListen failed\n";
    return false;
  }

  if (SimBricksBaseIfEstablish(&ests, 1)) {
    std::cerr << "PciIfInit: SimBricksBaseIfEstablish failed\n";
    return false;
  }

  return true;
}

bool h2d_read(volatile struct SimbricksProtoPcieH2DRead &read,
              uint64_t cur_ts) {
#if JPGD_DEBUG
  std::cout << "h2d_read ts=" << cur_ts << " bar=" << static_cast<int>(read.bar)
            << " offset=" << read.offset << " len=" << read.len << "\n";
#endif

  switch (read.bar) {
    case 0: {
      reg_read_write->issue_read(read.req_id, read.offset);
      break;
    }
    case 1: {
      if (read.offset + read.len > sizeof(verilator_regs)) {
        std::cerr
            << "error: read from verilator registers outside bounds offset="
            << read.offset << " len=" << read.len << "\n";
        return false;
      }

      volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
      volatile struct SimbricksProtoPcieD2HReadcomp *readcomp = &msg->readcomp;
      readcomp->req_id = read.req_id;
      std::memcpy(const_cast<uint8_t *>(readcomp->data),
                  reinterpret_cast<uint8_t *>(&verilator_regs) + read.offset,
                  read.len);
      SimbricksPcieIfD2HOutSend(&pcieif, msg,
                                SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);

      break;
    }
    default: {
      std::cerr << "error: read from unexpected bar " << read.bar << "\n";
      return false;
    }
  }
  return true;
}

bool h2d_write(volatile struct SimbricksProtoPcieH2DWrite &write,
               uint64_t cur_ts, bool posted) {
#if JPGD_DEBUG
  std::cout << "h2d_write ts=" << cur_ts
            << " bar=" << static_cast<int>(write.bar)
            << " offset=" << write.offset << " len=" << write.len << "\n";
#endif

  switch (write.bar) {
    case 0: {
      uint32_t data;
      if (write.len != 4) {
        throw "h2d_write() JPEG decoder register write needs to be 32 bits";
      }
      std::memcpy(&data, const_cast<uint8_t *>(write.data), write.len);
      reg_read_write->issue_write(write.req_id, write.offset, data, posted);
      break;
    }
    case 1: {
      if (write.offset + write.len > sizeof(verilator_regs)) {
        std::cerr
            << "error: write to verilator registers outside bounds offset="
            << write.offset << " len=" << write.len << "\n";
        return false;
      }
      std::memcpy((reinterpret_cast<uint8_t *>(&verilator_regs)) + write.offset,
                  const_cast<uint8_t *>(write.data), write.len);
      apply_ctrl_changes();
      break;
    }
    default: {
      std::cerr << "error: write to unexpected bar " << write.bar << "\n";
      return false;
    }
  }

  if (!posted) {
    volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(cur_ts);
    volatile struct SimbricksProtoPcieD2HWritecomp &writecomp = msg->writecomp;
    writecomp.req_id = write.req_id;

    SimbricksPcieIfD2HOutSend(&pcieif, msg,
                              SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
  }
  return true;
}

bool h2d_readcomp(volatile struct SimbricksProtoPcieH2DReadcomp &readcomp,
                  uint64_t cur_ts) {
  dma_read->read_done(readcomp.req_id, const_cast<uint8_t *>(readcomp.data));
  return true;
}

bool h2d_writecomp(volatile struct SimbricksProtoPcieH2DWritecomp &writecomp,
                   uint64_t cur_ts) {
  dma_write->write_done(writecomp.req_id);
  return true;
}

bool poll_h2d(uint64_t cur_ts) {
  volatile union SimbricksProtoPcieH2D *msg =
      SimbricksPcieIfH2DInPoll(&pcieif, cur_ts);

  // no msg available
  if (msg == nullptr)
    return true;

  uint8_t type = SimbricksPcieIfH2DInType(&pcieif, msg);

  switch (type) {
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READ:
      if (!h2d_read(msg->read, cur_ts)) {
        return false;
      }
      break;
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE:
      if (!h2d_write(msg->write, cur_ts, false)) {
        return false;
      }
      break;
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE_POSTED:
      if (!h2d_write(msg->write, cur_ts, true)) {
        return false;
      }
      break;
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP:
      if (!h2d_readcomp(msg->readcomp, cur_ts)) {
        return false;
      }
      break;
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP:
      if (!h2d_writecomp(msg->writecomp, cur_ts)) {
        return false;
      }
      break;
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_DEVCTRL:
    case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
      break; /* noop */
    case SIMBRICKS_PROTO_MSG_TYPE_TERMINATE:
      std::cerr << "poll_h2d: peer terminated\n";
      exiting = true;
      break;
    default:
      std::cerr << "warn: poll_h2d: unsupported type=" << type << "\n";
  }

  SimbricksPcieIfH2DInDone(&pcieif, msg);
  return true;
}

// react to changes of ctrl signals
void apply_ctrl_changes() {
  // change to tracing_active
  if (!tracing_active && verilator_regs.tracing_active) {
    tracing_active = true;
    std::ostringstream trace_file{};
    trace_file << trace_filename << "_" << trace_nr++ << ".vcd";
    trace->open(trace_file.str().c_str());
  } else if (tracing_active && !verilator_regs.tracing_active) {
    tracing_active = false;
    trace->close();
  }
}

int main(int argc, char **argv) {
  if (argc < 3 || argc > 7) {
    std::cerr
        << "Usage: jpeg_decoder_verilator PCI-SOCKET SHM START-TIMESTAMP-PS "
           "SYNC-PERIOD PCI-LATENCY TRACE-FILE\n";
    return EXIT_FAILURE;
  }

  struct SimbricksBaseIfParams if_params;
  std::memset(&if_params, 0, sizeof(if_params));
  SimbricksPcieIfDefaultParams(&if_params);

  cur_ts = strtoull(argv[3], nullptr, 0);
  if_params.sync_interval = strtoull(argv[4], nullptr, 0) * 1000ULL;
  if_params.link_latency = strtoull(argv[5], nullptr, 0) * 1000ULL;

  if_params.sock_path = argv[1];
  if (!PciIfInit(argv[2], if_params)) {
    return EXIT_FAILURE;
  }

  bool sync = SimbricksBaseIfSyncEnabled(&pcieif.base);
  signal(SIGINT, sigint_handler);
  signal(SIGUSR1, sigusr1_handler);

  trace_filename = std::string(argv[6]);

  // initialize
  top = std::make_unique<Vjpeg_decoder>();
  trace = std::make_unique<VerilatedVcdC>();
  Verilated::traceEverOn(true);
  top->trace(trace.get(), 0);

  dma_read = std::make_unique<JpegDecAXISubordinateRead>(*top);
  dma_write = std::make_unique<JpegDecAXISubordinateWrite>(*top);
  reg_read_write = std::make_unique<JpegDecAXILManager>(*top);

  // reset chip
  top->rst = 1;
  top->clk = 0;
  top->eval();
  top->clk = 1;
  top->eval();
  top->rst = 0;

  // main simulation loop
  while (!exiting) {
    // send required sync messages
    while (SimbricksPcieIfD2HOutSync(&pcieif, cur_ts) < 0) {
      // std::cerr << "warn: SimbricksPcieIfD2HOutSync failed cur_ts=" << cur_ts
      //           << "\n";
    }

    // process available incoming messages for current timestamp
    do {
      poll_h2d(cur_ts);
    } while (!exiting &&
             ((sync && SimbricksPcieIfH2DInTimestamp(&pcieif) <= cur_ts)));

    // falling edge
    top->clk = 0;
    top->eval();
    if (tracing_active) {
      trace->dump(cur_ts);
    }
    cur_ts += clock_period / 2;

    // evaluate on rising edge
    top->clk = 1;
    dma_read->step(cur_ts);
    dma_write->step(cur_ts);
    reg_read_write->step(cur_ts);
    top->eval();

    // finalize updates
    dma_read->step_apply();
    dma_write->step_apply();
    reg_read_write->step_apply();

    // write trace
    if (tracing_active) {
      trace->dump(cur_ts);
    }
    cur_ts += clock_period / 2;
  }

  top->final();
  trace = nullptr;
  top = nullptr;
  return 0;
}
