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

 #include <signal.h>
 #include <verilated_vcd_c.h>
 
 #include <algorithm>
 #include <iostream>
 #include <memory>
 
 #include <simbricks/axi/axi_s.hpp>
 #include <simbricks/axi/axi_subordinate.hh>
 #include <simbricks/axi/axil_manager.hh>
 
 #include "sims/external/corundum/obj_dir/Vmqnic_core_axi.h"
 
 extern "C" {
 #include <simbricks/nicif/nicif.h>
 }
 
 #define CORUNDUM_VERILATOR_DEBUG 1
 // #define CORUNDUM_VERILATOR_TRACE 1
 #define CORUNDUM_VERILATOR_TRACE_LEVEL 40
 
 /* **************************************************************************
  * signal handling
  * ************************************************************************** */
 
 static uint64_t main_time = 0;
 static volatile int exiting = 0;
 static void sigint_handler(int dummy) {
   exiting = 1;
 }
 static void sigusr1_handler(int dummy) {
   sim_log::LogError("main_time = %lu\n", main_time);
 }
 
 /* **************************************************************************
  * helpers
  * ************************************************************************** */
 
 void reset_corundum(Vmqnic_core_axi &top_verilator_interface) {
   top_verilator_interface->rx_rst = 0;
   top_verilator_interface.s_axis_rx_tvalid = 0;
   top_verilator_interface.s_axis_rx_tready = 0;
   top_verilator_interface.s_axis_rx_tdata = 0;
   top_verilator_interface.s_axis_rx_tkeep = 0;
   top_verilator_interface.s_axis_rx_tlast = 0;
   // top_verilator_interface.s_axis_rx_tuser = 0;
   
   top_verilator_interface->tx_rst = 0;
   top_verilator_interface.m_axis_tx_tvalid = 0;
   top_verilator_interface.m_axis_tx_tready = 0;
   top_verilator_interface.m_axis_tx_tdata = 0;
   top_verilator_interface.m_axis_tx_tkeep = 0;
   top_verilator_interface.m_axis_tx_tlast = 0;
   top_verilator_interface.m_axis_tx_tuser = 0;
 
   top_verilator_interface.m_axi_araddr = 0;
   top_verilator_interface.m_axi_arid = 0;
   top_verilator_interface.m_axi_arready = 0;
   top_verilator_interface.m_axi_arvalid = 0;
   top_verilator_interface.m_axi_arlen = 0;
   top_verilator_interface.m_axi_arsize = 0;
   top_verilator_interface.m_axi_arburst = 0;
   // top_verilator_interface.m_axi_rdata = 0;
   top_verilator_interface.m_axi_rid = 0;
   top_verilator_interface.m_axi_rready = 0;
   top_verilator_interface.m_axi_rvalid = 0;
   top_verilator_interface.m_axi_rlast = 0;
 
   top_verilator_interface.m_axi_awaddr = 0;
   top_verilator_interface.m_axi_awid = 0;
   top_verilator_interface.m_axi_awready = 0;
   top_verilator_interface.m_axi_awvalid = 0;
   top_verilator_interface.m_axi_awlen = 0;
   top_verilator_interface.m_axi_awsize = 0;
   top_verilator_interface.m_axi_awburst = 0;
   // top_verilator_interface.m_axi_wdata = 0;
   top_verilator_interface.m_axi_wready = 0;
   top_verilator_interface.m_axi_wvalid = 0;
   top_verilator_interface.m_axi_wstrb = 0;
   top_verilator_interface.m_axi_wlast = 0;
   top_verilator_interface.m_axi_bid = 0;
   top_verilator_interface.m_axi_bready = 0;
   top_verilator_interface.m_axi_bvalid = 0;
   top_verilator_interface.m_axi_bresp = 0;
 
   top_verilator_interface.s_axil_ctrl_araddr = 0;
   top_verilator_interface.s_axil_ctrl_arready = 0;
   top_verilator_interface.s_axil_ctrl_arvalid = 0;
   top_verilator_interface.s_axil_ctrl_rdata = 0;
   top_verilator_interface.s_axil_ctrl_rready = 0;
   top_verilator_interface.s_axil_ctrl_rvalid = 0;
   top_verilator_interface.s_axil_ctrl_rresp = 0;
   top_verilator_interface.s_axil_ctrl_awaddr = 0;
   top_verilator_interface.s_axil_ctrl_awready = 0;
   top_verilator_interface.s_axil_ctrl_awvalid = 0;
   top_verilator_interface.s_axil_ctrl_wdata = 0;
   top_verilator_interface.s_axil_ctrl_wready = 0;
   top_verilator_interface.s_axil_ctrl_wvalid = 0;
   top_verilator_interface.s_axil_ctrl_wstrb = 0;
   top_verilator_interface.s_axil_ctrl_bready = 0;
   top_verilator_interface.s_axil_ctrl_bvalid = 0;
   top_verilator_interface.s_axil_ctrl_bresp = 0;
 
   top_verilator_interface.rst = 0;
   top_verilator_interface.clk = 0;
   top_verilator_interface.tx_clk = 0;
   top_verilator_interface.rx_clk = 0;
   top_verilator_interface.ptp_clk = 0;
   top_verilator_interface.tx_ptp_clk = 0;
   top_verilator_interface.rx_ptp_clk = 0;
   top_verilator_interface.tx_status = 0;
   top_verilator_interface.rx_status = 0;
 }
 
 static volatile union SimbricksProtoPcieD2H *d2h_alloc(
     struct SimbricksNicIf &nicif, uint64_t cur_ts) {
   return SimbricksPcieIfD2HOutAlloc(&nicif.pcie, cur_ts);
 }
 
 /* **************************************************************************
  * AXI, AXIL and AXIS interface definitions required by this adapter
  * ************************************************************************** */
 
 // handles DMA read requests
 class CorundumAXISubordinateRead
     : public simbricks::AXISubordinateRead<
           4, 1, 16,
           /*num concurrently pending requests*/ 16> {
  public:
   explicit CorundumAXISubordinateRead(struct SimbricksNicIf &nicif,
                                       Vmqnic_core_axi &top_verilator_interface)
       : AXISubordinateRead(
             reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_araddr),
             &top_verilator_interface.m_axi_arid,
             top_verilator_interface.m_axi_arready,
             top_verilator_interface.m_axi_arvalid,
             top_verilator_interface.m_axi_arlen,
             top_verilator_interface.m_axi_arsize,
             top_verilator_interface.m_axi_arburst,
             reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_rdata),
             &top_verilator_interface.m_axi_rid,
             top_verilator_interface.m_axi_rready,
             top_verilator_interface.m_axi_rvalid,
             top_verilator_interface.m_axi_rlast),
         nicif_(nicif) {
   }
 
  private:
   struct SimbricksNicIf &nicif_;
 
   void do_read(const simbricks::AXIOperation &axi_op) final {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "CorundumAXISubordinateRead::doRead() ts=%lu id=%lu addr=0x%lx "
         "len=%lu\n",
         main_time, axi_op.id, axi_op.addr, axi_op.len);
 #endif
 
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif_, main_time);
     if (not msg) {
       sim_log::LogError(
           "CorundumAXISubordinateRead::doRead() dma read alloc failed\n");
       std::terminate();
     }
 
     unsigned int max_size = SimbricksPcieIfH2DOutMsgLen(&nicif_.pcie) -
                             sizeof(SimbricksProtoPcieH2DReadcomp);
     if (axi_op.len > max_size) {
       sim_log::LogError(
           "CorundumAXISubordinateRead error: read data of length=%lu doesn't "
           "fit into a SimBricks "
           "message\n",
           axi_op.len);
       std::terminate();
     }
 
     volatile struct SimbricksProtoPcieD2HRead *read = &msg->read;
     read->req_id = axi_op.id;
     read->offset = axi_op.addr;
     read->len = axi_op.len;
     SimbricksPcieIfD2HOutSend(&nicif_.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_READ);
   }
 };
 
 // handles DMA write requests
 class CorundumAXISubordinateWrite
     : public simbricks::AXISubordinateWrite<
           4, 1, 16,
           /*num concurrently pending requests*/ 16> {
  public:
   explicit CorundumAXISubordinateWrite(struct SimbricksNicIf &nicif,
                                        Vmqnic_core_axi &top_verilator_interface)
       : AXISubordinateWrite(
             reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_awaddr),
             &top_verilator_interface.m_axi_awid,
             top_verilator_interface.m_axi_awready,
             top_verilator_interface.m_axi_awvalid,
             top_verilator_interface.m_axi_awlen,
             top_verilator_interface.m_axi_awsize,
             top_verilator_interface.m_axi_awburst,
             reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_wdata),
             top_verilator_interface.m_axi_wready,
             top_verilator_interface.m_axi_wvalid,
             top_verilator_interface.m_axi_wstrb,
             top_verilator_interface.m_axi_wlast,
             &top_verilator_interface.m_axi_bid,
             top_verilator_interface.m_axi_bready,
             top_verilator_interface.m_axi_bvalid,
             top_verilator_interface.m_axi_bresp),
         nicif_(nicif) {
   }
 
  private:
   struct SimbricksNicIf &nicif_;
 
   void do_write(const simbricks::AXIOperation &axi_op) final {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "CorundumAXISubordinateWrite::doWrite() ts=%lu id=%lu addr=0x%lx "
         "len=%lu data=",
         main_time, axi_op.id, axi_op.addr, axi_op.len);
     std::for_each_n(axi_op.buf.get(), axi_op.len,
                     [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
     fputs("\n", stdout);
 #endif
 
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif_, main_time);
     if (not msg) {
       sim_log::LogError(
           "CorundumAXISubordinateWrite::doWrite() dma read alloc failed\n");
       std::terminate();
     }
 
     volatile struct SimbricksProtoPcieD2HWrite *write = &msg->write;
     unsigned int max_size =
         SimbricksPcieIfH2DOutMsgLen(&nicif_.pcie) - sizeof(*write);
     if (axi_op.len > max_size) {
       sim_log::LogError(
           "CorundumAXISubordinateWrite error: write data of length=%lu doesn't "
           "fit into a SimBricks "
           "message\n",
           axi_op.len);
       std::terminate();
     }
 
     write->req_id = axi_op.id;
     write->offset = axi_op.addr;
     write->len = axi_op.len;
     std::memcpy(const_cast<uint8_t *>(write->data), axi_op.buf.get(),
                 axi_op.len);
     SimbricksPcieIfD2HOutSend(&nicif_.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITE);
   }
 };
 
 // handles host to device register reads / writes
 class CorundumAXILManager : public simbricks::AXILManager<3, 4> {
  public:
   explicit CorundumAXILManager(struct SimbricksNicIf &nicif,
                                Vmqnic_core_axi &top_verilator_interface)
       : AXILManager(reinterpret_cast<uint8_t *>(
                         &top_verilator_interface.s_axil_ctrl_araddr),
                     top_verilator_interface.s_axil_ctrl_arready,
                     top_verilator_interface.s_axil_ctrl_arvalid,
                     reinterpret_cast<uint8_t *>(
                         &top_verilator_interface.s_axil_ctrl_rdata),
                     top_verilator_interface.s_axil_ctrl_rready,
                     top_verilator_interface.s_axil_ctrl_rvalid,
                     top_verilator_interface.s_axil_ctrl_rresp,
                     reinterpret_cast<uint8_t *>(
                         &top_verilator_interface.s_axil_ctrl_awaddr),
                     top_verilator_interface.s_axil_ctrl_awready,
                     top_verilator_interface.s_axil_ctrl_awvalid,
                     reinterpret_cast<uint8_t *>(
                         &top_verilator_interface.s_axil_ctrl_wdata),
                     top_verilator_interface.s_axil_ctrl_wready,
                     top_verilator_interface.s_axil_ctrl_wvalid,
                     top_verilator_interface.s_axil_ctrl_wstrb,
                     top_verilator_interface.s_axil_ctrl_bready,
                     top_verilator_interface.s_axil_ctrl_bvalid,
                     top_verilator_interface.s_axil_ctrl_bresp),
         nicif_(nicif) {
   }
 
  private:
   struct SimbricksNicIf &nicif_;
 
   void read_done(simbricks::AXILOperationR &axi_op) final {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "CorundumAXILManager::read_done() ts=%lu  id=%lu  addr=0x%lx data=",
         main_time, axi_op.req_id, axi_op.addr);
     std::for_each_n((uint8_t *)(&axi_op.data), sizeof(axi_op.data),
                     [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
     fputs("\n", stdout);
 #endif
 
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif_, main_time);
     if (not msg) {
       sim_log::LogError(
           "CorundumAXILManager::read_done() completion alloc failed\n");
       std::terminate();
     }
 
     volatile struct SimbricksProtoPcieD2HReadcomp *readcomp = &msg->readcomp;
     std::memcpy(const_cast<uint8_t *>(readcomp->data), &axi_op.data, 4);
     readcomp->req_id = axi_op.req_id;
     SimbricksPcieIfD2HOutSend(&nicif_.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
   }
 
   void write_done(simbricks::AXILOperationW &axi_op) final {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "CorundumAXILManager::write_done() ts=%lu  id=%lu  addr=0x%lx "
         "posted=%d data=",
         main_time, axi_op.req_id, axi_op.addr, axi_op.posted);
     std::for_each_n((uint8_t *)(&axi_op.data), sizeof(axi_op.data),
                     [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
     fputs("\n", stdout);
 #endif
 
     if (axi_op.posted) {
       return;
     }
 
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif_, main_time);
     if (not msg) {
       sim_log::LogError(
           "CorundumAXILManager::write_done() completion alloc failed\n");
       std::terminate();
     }
 
     volatile struct SimbricksProtoPcieD2HWritecomp *writecomp = &msg->writecomp;
     writecomp->req_id = axi_op.req_id;
     SimbricksPcieIfD2HOutSend(&nicif_.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
   }
 };
 
 // handles network interfacing AXI stream interface
 using AxiSFromNetworkT = simbricks::AXISManager<8, 32, 2048>;
 using AxiSToNetworkT = simbricks::AXISSubordinate<8, 2048>;
 
 /* **************************************************************************
  * H2D handling methods
  * ************************************************************************** */
 
 void h2d_read(volatile struct SimbricksProtoPcieH2DRead &read, uint64_t cur_ts,
               CorundumAXILManager &mmio) {
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo("h2d_read ts=%lu bar=%d offset=0x%lx len=%lu\n", cur_ts,
                    static_cast<int>(read.bar),
                    static_cast<uint64_t>(read.offset),
                    static_cast<uint64_t>(read.len));
 #endif
 
   if (read.bar != 0) {
     sim_log::LogError("write to unexpected bar=%d", static_cast<int>(read.bar));
     std::terminate();
   }
 
   mmio.issue_read(read.req_id, read.offset);
 }
 
 void h2d_write(struct SimbricksNicIf &nicif,
                volatile struct SimbricksProtoPcieH2DWrite &write,
                uint64_t cur_ts, bool posted, CorundumAXILManager &mmio) {
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo(
       "h2d_write ts=%lu bar=%d offset=0x%lx len=%lu posted=%d data=", cur_ts,
       static_cast<int>(write.bar), static_cast<uint64_t>(write.offset),
       static_cast<uint64_t>(write.len), posted);
   std::for_each_n(const_cast<uint8_t *>(write.data), write.len,
                   [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
   fputs("\n", stdout);
 #endif
 
   if (write.bar != 0) {
     sim_log::LogError("write to unexpected bar=%d", write.bar);
     std::terminate();
   }
 
   if (write.len != 4) {
     sim_log::LogError(
         "h2d_write() Corundum register write only supported up to 8 bytes\n");
     std::terminate();
   }
 
   uint32_t data = 0;
   std::memcpy(&data, const_cast<uint8_t *>(write.data), write.len);
   mmio.issue_write(write.req_id, write.offset, data, posted);
 
   if (!posted) {
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif, cur_ts);
     volatile struct SimbricksProtoPcieD2HWritecomp &writecomp = msg->writecomp;
     writecomp.req_id = write.req_id;
 
     SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
   }
   return;
 }
 
 void h2d_readcomp(volatile struct SimbricksProtoPcieH2DReadcomp &readcomp,
                   uint64_t cur_ts, CorundumAXISubordinateRead &dma_read) {
   dma_read.read_done(readcomp.req_id, const_cast<uint8_t *>(readcomp.data));
 }
 
 void h2d_writecomp(volatile struct SimbricksProtoPcieH2DWritecomp &writecomp,
                    uint64_t cur_ts, CorundumAXISubordinateWrite &dma_write) {
   dma_write.write_done(writecomp.req_id);
 }
 
 void poll_h2d(struct SimbricksNicIf &nicif, uint64_t cur_ts,
               CorundumAXISubordinateRead &dma_read,
               CorundumAXISubordinateWrite &dma_write,
               CorundumAXILManager &mmio) {
   volatile union SimbricksProtoPcieH2D *msg =
       SimbricksPcieIfH2DInPoll(&nicif.pcie, cur_ts);
 
   if (msg == NULL) {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     // sim_log::LogWarn("poll_h2d msg NULL\n");
 #endif
     return;
   }
 
   uint8_t type = SimbricksPcieIfH2DInType(&nicif.pcie, msg);
   switch (type) {
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_READ:
       h2d_read(msg->read, cur_ts, mmio);
       break;
 
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE:
       h2d_write(nicif, msg->write, cur_ts, false, mmio);
       break;
 
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE_POSTED:
       h2d_write(nicif, msg->write, cur_ts, true, mmio);
       break;
 
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP:
       h2d_readcomp(msg->readcomp, cur_ts, dma_read);
       break;
 
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP:
       h2d_writecomp(msg->writecomp, cur_ts, dma_write);
       break;
 
     case SIMBRICKS_PROTO_PCIE_H2D_MSG_DEVCTRL:
     case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
       break;
 
     case SIMBRICKS_PROTO_MSG_TYPE_TERMINATE:
       sim_log::LogError("poll_h2d: peer terminated\n");
       break;
 
     default:
       sim_log::LogError("poll_h2d: unsupported type=%d", type);
   }
 
   SimbricksPcieIfH2DInDone(&nicif.pcie, msg);
 }
 
 /* **************************************************************************
  * N2D handling methods
  * ************************************************************************** */
 
 void n2d_recv(volatile struct SimbricksProtoNetMsgPacket &packet,
               uint64_t cur_ts, AxiSFromNetworkT &axis_from_network) {
   if (axis_from_network.full()) {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogError("corundum verilator n2d_recv: dropping packet\n");
 #endif
     return;
   }
 
   // NOTE: const_cast ing the member of the volatile struct to non-volatile is
   // undefined behavior...
   axis_from_network.read(const_cast<const uint8_t *>(packet.data), packet.len);
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo(
       "%lu corundum verilator n2d_recv received packet: packet data=", cur_ts);
   std::for_each_n(const_cast<uint8_t *>(packet.data), packet.len,
                   [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
   fputs("\n", stdout);
 #endif
 }
 
 void poll_n2d(struct SimbricksNicIf &nicif, uint64_t cur_ts,
               AxiSFromNetworkT &axis_from_network) {
   volatile union SimbricksProtoNetMsg *msg =
       SimbricksNetIfInPoll(&nicif.net, cur_ts);
 
   if (msg == NULL) {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     // sim_log::LogWarn("poll_n2d msg NULL\n");
 #endif
     return;
   }
 
   uint8_t type = SimbricksNetIfInType(&nicif.net, msg);
   switch (type) {
     case SIMBRICKS_PROTO_NET_MSG_PACKET:
       n2d_recv(msg->packet, cur_ts, axis_from_network);
       break;
 
     case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
       break;
 
     default:
       sim_log::LogError("poll_n2d: unsupported type=%d\n", type);
   }
 
   SimbricksNetIfInDone(&nicif.net, msg);
 }
 
 void packet_d2n(struct SimbricksNicIf &nicif, uint64_t cur_ts,
                 AxiSToNetworkT &axis_to_network) {
   if (not axis_to_network.is_packet_done()) {
     // if no packet is done we have nothing to do
     return;
   }
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo(
       "%lu corundum verilator packet_d2n try packet transmission\n", cur_ts);
 #endif
 
   volatile union SimbricksProtoNetMsg *msg =
       SimbricksNetIfOutAlloc(&nicif.net, cur_ts);
   if (not msg) {
     sim_log::LogError("corundum packet_d2n msg allocation failed");
     std::terminate();
   }
 
   volatile struct SimbricksProtoNetMsgPacket *packet = &msg->packet;
   size_t packet_len = 0;
   axis_to_network.write(const_cast<uint8_t *>(packet->data), &packet_len);
   if (packet_len > UINT16_MAX) {
     sim_log::LogError("corundum packet_d2n packet len too large\n");
     std::terminate();
   }
   packet->len = static_cast<uint16_t>(packet_len);
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo(
       "%lu corundum verilator packet_d2n transmitted packet: packet len=%lu, "
       "packet data=",
       cur_ts, packet_len);
   std::for_each_n(const_cast<uint8_t *>(packet->data), packet->len,
                   [](uint8_t &val) { fprintf(stdout, "%02X ", val); });
   fputs("\n", stdout);
 #endif
   SimbricksNetIfOutSend(&nicif.net, msg, SIMBRICKS_PROTO_NET_MSG_PACKET);
 }
 
 /* **************************************************************************
  * interrupt handling
  * ************************************************************************** */
 
 class MsiInterruptHandler {
   const uint32_t &msi_irq_;
   struct SimbricksNicIf &nicif_;
 
   void msi_issue(uint32_t intr_vec, uint64_t main_time) const {
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "MsiInterruptHandler::msi_issue: MSI interrupt ts=%lu, vec=%lu\n",
         main_time, intr_vec);
 #endif
 
     volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif_, main_time);
     if (not msg) {
       sim_log::LogError(
           "MsiInterruptHandler::msi_issue: unable to allocate msg\n");
       std::terminate();
     }
 
     volatile struct SimbricksProtoPcieD2HInterrupt *intr = &msg->interrupt;
     intr->vector = intr_vec;
     intr->inttype = SIMBRICKS_PROTO_PCIE_INT_MSI;
 
     SimbricksPcieIfD2HOutSend(&nicif_.pcie, msg,
                               SIMBRICKS_PROTO_PCIE_D2H_MSG_INTERRUPT);
   }
 
   uint32_t get_intr_bit_pos_i(uint32_t i) const {
     uint32_t res = (1ULL << i) & msi_irq_;
     return res;
   }
 
  public:
   MsiInterruptHandler(struct SimbricksNicIf &nicif, const uint32_t &msi_irq)
       : msi_irq_(msi_irq), nicif_(nicif) {
   }
 
   void step(uint64_t main_time) {
     if (not msi_irq_) {
       return;
     }
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
     sim_log::LogInfo(
         "MsiInterruptHandler::step: MSI interrupt ts=%lu, msi_irq=%lu\n",
         main_time, msi_irq_);
 #endif
 
     for (uint32_t i = 0; i < 32; i++) {
       uint32_t bit = get_intr_bit_pos_i(i);
       if (not static_cast<bool>(bit)) {
         continue;
       }
 
       msi_issue(i, main_time);
     }
   }
 };
 
 /* **************************************************************************
  * main adapter driver
  * ************************************************************************** */
 
 int main(int argc, char *argv[]) {
   // declarations
   auto top_verilator_interface = std::make_unique<Vmqnic_core_axi>();
   auto trace = std::make_unique<VerilatedVcdC>();
 
   struct SimbricksBaseIfParams netParams;
   struct SimbricksBaseIfParams pcieParams;
   struct SimbricksNicIf nicif;
   struct SimbricksProtoPcieDevIntro di;
   uint64_t clock_period = 4 * 1000ULL;  // 4ns -> 250MHz
 
   AxiSFromNetworkT axis_from_network{
       top_verilator_interface->s_axis_rx_tvalid,
       top_verilator_interface->s_axis_rx_tready,
       reinterpret_cast<uint8_t *>(&top_verilator_interface->s_axis_rx_tdata),
       &top_verilator_interface->s_axis_rx_tkeep,
       top_verilator_interface->s_axis_rx_tlast,
       reinterpret_cast<uint8_t *>(&top_verilator_interface->s_axis_rx_tuser)};
 
   AxiSToNetworkT axis_to_network{
       top_verilator_interface->m_axis_tx_tvalid,
       top_verilator_interface->m_axis_tx_tready,
       reinterpret_cast<uint8_t *>(&top_verilator_interface->m_axis_tx_tdata),
       &top_verilator_interface->m_axis_tx_tkeep,
       top_verilator_interface->m_axis_tx_tlast,
       reinterpret_cast<uint8_t *>(&top_verilator_interface->m_axis_tx_tuser)};
 
   CorundumAXISubordinateRead dma_read{nicif, *top_verilator_interface};
   CorundumAXISubordinateWrite dma_write{nicif, *top_verilator_interface};
   CorundumAXILManager mmio{nicif, *top_verilator_interface};
 
   MsiInterruptHandler msi_intr_handler{nicif, top_verilator_interface->irq};
 
   // argument parsing and initialization
   if (argc < 4 || argc > 10) {
     sim_log::LogError(
         "Usage: corundum_simbricks_adapter PCI-SOCKET ETH-SOCKET "
         "SHM [SYNC-MODE] [START-TICK] [SYNC-PERIOD] [PCI-LATENCY] "
         "[ETH-LATENCY] [CLOCK-FREQ-MHZ]\n");
     return EXIT_FAILURE;
   }
 
   if (argc >= 6) {
     main_time = strtoull(argv[5], NULL, 0);
   }
   if (argc >= 7) {
     uint64_t sync_interval = strtoull(argv[6], NULL, 0) * 1000ULL;
     netParams.sync_interval = sync_interval;
     pcieParams.sync_interval = sync_interval;
   }
   if (argc >= 8) {
     pcieParams.link_latency = strtoull(argv[7], NULL, 0) * 1000ULL;
   }
   if (argc >= 9) {
     netParams.link_latency = strtoull(argv[8], NULL, 0) * 1000ULL;
   }
   if (argc >= 10) {
     clock_period = 1000000ULL / strtoull(argv[9], NULL, 0);
   }
 
   SimbricksNetIfDefaultParams(&netParams);
   SimbricksPcieIfDefaultParams(&pcieParams);
 
   pcieParams.sock_path = argv[1];
   netParams.sock_path = argv[2];
 
   memset(&di, 0, sizeof(di));
   di.bars[0].len = 1 << 24;
   di.bars[0].flags = SIMBRICKS_PROTO_PCIE_BAR_64;
   di.pci_vendor_id = 0x5543;
   di.pci_device_id = 0x1001;
   di.pci_class = 0x02;
   di.pci_subclass = 0x00;
   di.pci_revision = 0x00;
   di.pci_msi_nvecs = 32;
 
   sim_log::Logger::GetRegistry().SetFlush(true);
 
   if (SimbricksNicIfInit(&nicif, argv[3], &netParams, &pcieParams, &di)) {
     sim_log::LogError("corundum simbricks adapter SimbricksNicIfInit failed\n");
     return EXIT_FAILURE;
   }
 
   bool sync_pci = SimbricksBaseIfSyncEnabled(&nicif.pcie.base);
   bool sync_eth = SimbricksBaseIfSyncEnabled(&nicif.net.base);
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogRegistry().SetFlush(true);
   sim_log::LogInfo("sync_pci=%d sync_eth=%d\n", sync_pci, sync_eth);
 #endif
 
   signal(SIGINT, sigint_handler);
   signal(SIGUSR1, sigusr1_handler);
 
 #ifdef CORUNDUM_VERILATOR_TRACE
   Verilated::traceEverOn(true);
   top_verilator_interface->trace(trace.get(), CORUNDUM_VERILATOR_TRACE_LEVEL);
   trace->open("corundum-verilator-debug.vcd");
 #endif
 
   reset_corundum(*top_verilator_interface);
   top_verilator_interface->rst = 1;
   top_verilator_interface->tx_rst = 1;
   top_verilator_interface->rx_rst = 1;
   top_verilator_interface->eval();
 
   /* raising edge */
   top_verilator_interface->clk = 1;
   top_verilator_interface->eval();
   top_verilator_interface->rst = 0;
   top_verilator_interface->tx_rst = 0;
   top_verilator_interface->rx_rst = 0;
   top_verilator_interface->tx_status = 1;
   top_verilator_interface->rx_status = 1;
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo("corundum start main simulation loop\n");
 #endif
 
   // main simulation loop
   while (not exiting) {
     while (SimbricksNicIfSync(&nicif, main_time) < 0) {
       sim_log::LogWarn(
           "SimbricksPcieIfD2HOutSync or SimbricksNetIfOutSync failed (t=%lu)\n",
           main_time);
     }
 
     do {
       poll_h2d(nicif, main_time, dma_read, dma_write, mmio);
       poll_n2d(nicif, main_time, axis_from_network);
     } while (
         not exiting and
         ((sync_pci and
           SimbricksPcieIfH2DInTimestamp(&nicif.pcie) <= main_time) or
          (sync_eth and SimbricksNetIfInTimestamp(&nicif.net) <= main_time)));
 
     /* falling edge */
     top_verilator_interface->clk = 0;
     top_verilator_interface->tx_clk = 0;
     top_verilator_interface->rx_clk = 0;
     // top_verilator_interface->ptp_clk = 0;
     // top_verilator_interface->tx_ptp_clk = 0;
     // top_verilator_interface->rx_ptp_clk = 0;
     top_verilator_interface->eval();
 #ifdef CORUNDUM_VERILATOR_TRACE
     trace->dump(main_time);
 #endif
     main_time += clock_period / 2;
 
     // evaluate on rising edge
     if (top_verilator_interface->tx_status == 0)
       sim_log::LogInfo("corundum-verilator-nic tx_status %lu\n", top_verilator_interface->tx_status);
     top_verilator_interface->clk = 1;
     top_verilator_interface->tx_clk = 1;
     top_verilator_interface->rx_clk = 1;
     // top_verilator_interface->ptp_clk = 1;
     // top_verilator_interface->tx_ptp_clk = 1;
     // top_verilator_interface->rx_ptp_clk = 1;
     dma_read.step(main_time);
     dma_write.step(main_time);
     mmio.step(main_time);
     axis_from_network.step();
     axis_to_network.step();
     packet_d2n(nicif, main_time, axis_to_network);
     msi_intr_handler.step(main_time);
     top_verilator_interface->eval();
 
     //  finalize updates
     dma_read.step_apply();
     dma_write.step_apply();
     mmio.step_apply();
 
 #ifdef CORUNDUM_VERILATOR_TRACE
     trace->dump(main_time);
 #endif
     main_time += clock_period / 2;
   }
 
 #ifdef CORUNDUM_VERILATOR_TRACE
   trace->dump(main_time + 1);
   trace->close();
 #endif
 
   top_verilator_interface->final();
 
 #ifdef CORUNDUM_VERILATOR_DEBUG
   sim_log::LogInfo("corundum simbricks adapter finished\n");
 #endif
   return EXIT_SUCCESS;
 }