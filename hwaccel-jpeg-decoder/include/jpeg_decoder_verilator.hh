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
#pragma once
#include <Vjpeg_decoder.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#include <cstdint>

#include <simbricks/axi/axi_subordinate.hh>
#include <simbricks/axi/axil_manager.hh>

// handles DMA read requests
using AXISubordinateReadT =
    simbricks::AXISubordinateRead<4, 1, 4,
                                  /*num concurrently pending requests*/ 16>;
class JpegDecAXISubordinateRead : public AXISubordinateReadT {
public:
  explicit JpegDecAXISubordinateRead(Vjpeg_decoder &top)
      : AXISubordinateReadT(
            reinterpret_cast<uint8_t *>(&top.m_axi_araddr), &top.m_axi_arid,
            top.m_axi_arready, top.m_axi_arvalid, top.m_axi_arlen, axi_size_,
            top.m_axi_arburst, reinterpret_cast<uint8_t *>(&top.m_axi_rdata),
            &top.m_axi_rid, top.m_axi_rready, top.m_axi_rvalid,
            top.m_axi_rlast) {}

private:
  void do_read(const simbricks::AXIOperation &axi_op) final;

  CData axi_size_ = 0b010;
};

// handles DMA write requests
using AXISubordinateWriteT =
    simbricks::AXISubordinateWrite<4, 1, 4,
                                   /*num concurrently pending requests*/ 16>;
class JpegDecAXISubordinateWrite : public AXISubordinateWriteT {
public:
  explicit JpegDecAXISubordinateWrite(Vjpeg_decoder &top)
      : AXISubordinateWriteT(
            reinterpret_cast<uint8_t *>(&top.m_axi_awaddr), &top.m_axi_awid,
            top.m_axi_awready, top.m_axi_awvalid, top.m_axi_awlen, axi_size_,
            top.m_axi_awburst, reinterpret_cast<uint8_t *>(&top.m_axi_wdata),
            top.m_axi_wready, top.m_axi_wvalid, top.m_axi_wstrb,
            top.m_axi_wlast, &top.m_axi_bid, top.m_axi_bready, top.m_axi_bvalid,
            top.m_axi_bresp) {}

private:
  void do_write(const simbricks::AXIOperation &axi_op) final;

  CData axi_size_ = 0b010;
};

// handles host to device register reads / writes
using AXILManagerT = simbricks::AXILManager<4, 4>;
class JpegDecAXILManager : public AXILManagerT {
public:
  explicit JpegDecAXILManager(Vjpeg_decoder &top)
      : AXILManagerT(
            reinterpret_cast<uint8_t *>(&top.s_axil_araddr), top.s_axil_arready,
            top.s_axil_arvalid, reinterpret_cast<uint8_t *>(&top.s_axil_rdata),
            top.s_axil_rready, top.s_axil_rvalid, top.s_axil_rresp,
            reinterpret_cast<uint8_t *>(&top.s_axil_awaddr), top.s_axil_awready,
            top.s_axil_awvalid, reinterpret_cast<uint8_t *>(&top.s_axil_wdata),
            top.s_axil_wready, top.s_axil_wvalid, top.s_axil_wstrb,
            top.s_axil_bready, top.s_axil_bvalid, top.s_axil_bresp) {}

private:
  void read_done(simbricks::AXILOperationR &axi_op) final;
  void write_done(simbricks::AXILOperationW &axi_op) final;
};

extern "C" void sigint_handler(int dummy);
extern "C" void sigusr1_handler(int dummy);

bool PciIfInit(const char *shm_path,
               struct SimbricksBaseIfParams &baseif_params);

volatile union SimbricksProtoPcieD2H *d2h_alloc(uint64_t cur_ts);
bool h2d_read(volatile struct SimbricksProtoPcieH2DRead &read, uint64_t cur_ts);
bool h2d_write(volatile struct SimbricksProtoPcieH2DWrite &write,
               uint64_t cur_ts, bool posted);
bool h2d_readcomp(volatile struct SimbricksProtoPcieH2DReadcomp &readcomp,
                  uint64_t cur_ts);
bool h2d_writecomp(volatile struct SimbricksProtoPcieH2DWritecomp &writecomp,
                   uint64_t cur_ts);
bool poll_h2d(uint64_t cur_ts);

void apply_ctrl_changes();
