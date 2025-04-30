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

#include <simbricks/axi/axi_subordinate.hh>
#include <simbricks/axi/axil_manager.hh>

#include "obj_dir/VVTAShell.h"

#define AXIL_BYTES_DATA 4

// handles DMA read requests
using AXISubordinateReadT = simbricks::AXISubordinateRead<8, 1, 8, 16>;
class VTAAXISubordinateRead : public AXISubordinateReadT {
 public:
  explicit VTAAXISubordinateRead(VVTAShell &top)
      : AXISubordinateReadT(
            reinterpret_cast<uint8_t *>(&top.io_mem_ar_bits_addr),
            &top.io_mem_ar_bits_id, top.io_mem_ar_ready, top.io_mem_ar_valid,
            top.io_mem_ar_bits_len, top.io_mem_ar_bits_size,
            top.io_mem_ar_bits_burst,
            reinterpret_cast<uint8_t *>(&top.io_mem_r_bits_data),
            &top.io_mem_r_bits_id, top.io_mem_r_ready, top.io_mem_r_valid,
            top.io_mem_r_bits_last) {
  }

 private:
  void do_read(const simbricks::AXIOperation &axi_op) final;
};

// handles DMA write requests
using AXISubordinateWriteT = simbricks::AXISubordinateWrite<8, 1, 8, 16>;
class VTAAXISubordinateWrite : public AXISubordinateWriteT {
 public:
  explicit VTAAXISubordinateWrite(VVTAShell &top)
      : AXISubordinateWriteT(
            reinterpret_cast<uint8_t *>(&top.io_mem_aw_bits_addr),
            &top.io_mem_aw_bits_id, top.io_mem_aw_ready, top.io_mem_aw_valid,
            top.io_mem_aw_bits_len, top.io_mem_aw_bits_size,
            top.io_mem_aw_bits_burst,
            reinterpret_cast<uint8_t *>(&top.io_mem_w_bits_data),
            top.io_mem_w_ready, top.io_mem_w_valid, top.io_mem_w_bits_strb,
            top.io_mem_w_bits_last, &top.io_mem_b_bits_id, top.io_mem_b_ready,
            top.io_mem_b_valid, top.io_mem_b_bits_resp) {
  }

 private:
  void do_write(const simbricks::AXIOperation &axi_op) final;
};

// handles host to device register reads / writes
using AXILManagerT = simbricks::AXILManager<2, AXIL_BYTES_DATA>;
class VTAAXILManager : public AXILManagerT {
 public:
  explicit VTAAXILManager(VVTAShell &top)
      : AXILManagerT(
            reinterpret_cast<uint8_t *>(&top.io_host_ar_bits_addr),
            top.io_host_ar_ready, top.io_host_ar_valid,
            reinterpret_cast<uint8_t *>(&top.io_host_r_bits_data),
            top.io_host_r_ready, top.io_host_r_valid, top.io_host_r_bits_resp,
            reinterpret_cast<uint8_t *>(&top.io_host_aw_bits_addr),
            top.io_host_aw_ready, top.io_host_aw_valid,
            reinterpret_cast<uint8_t *>(&top.io_host_w_bits_data),
            top.io_host_w_ready, top.io_host_w_valid, top.io_host_w_bits_strb,
            top.io_host_b_ready, top.io_host_b_valid, top.io_host_b_bits_resp) {
  }

 private:
  void read_done(simbricks::AXILOperationR &axi_op) final;
  void write_done(simbricks::AXILOperationW &axi_op) final;
};