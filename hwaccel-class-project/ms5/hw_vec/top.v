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

module top #(parameter
  /* Actual multiplication matrix size: MUL_SIZE x MUL_SIZE */
  MUL_SIZE = 8,
  /** MMIO data width */
  MMIO_WIDTH = 32,
  /** MMIO address width */
  MMIO_ADDRBITS = 32,
  /** DMA data width */
  DMA_WIDTH = MUL_SIZE * 8,
  /** DMA address width */
  DMA_ADDRBITS = 32
) (
  /* clock pin: wiggles up and down every cycle */
  input clk,
  /* reset: set to 1 to initialize and then held at 0 */
  input rst,

  /* MMIO write request: set to 1 to indicate value should be written */
  input   mmio_w_req,
  /* MMIO register address to write */
  input   [MMIO_ADDRBITS-1:0] mmio_w_addr,
  /* verilator lint_off UNUSED */
  /* MMIO data to write */
  input   [MMIO_WIDTH-1:0] mmio_w_data,
  /* verilator lint_on UNUSED */

  /** MMIO read request */
  input   mmio_r_req,
  /** MMIO register address to read */
  input   [MMIO_ADDRBITS-1:0] mmio_r_addr,
  /** MMIO data that was read */
  output  [MMIO_WIDTH-1:0] mmio_r_data,

  /* Same for DMA. Note actual DMA engine is external. This external engine
     manages the dma control registers, and takes data and puts it into this
     write ports/grabs it from the read ports here. */
  input   dma_w_req,
  input   [DMA_ADDRBITS-1:0] dma_w_addr,
  input   [DMA_WIDTH-1:0] dma_w_data,

  input   dma_r_req,
  input   [DMA_ADDRBITS-1:0] dma_r_addr,
  output  [DMA_WIDTH-1:0] dma_r_data
);

  /* dma offsets for the matrix memories */
  parameter OFF_INA = 512*512;
  parameter OFF_INB = 2 * 512*512;
  parameter OFF_OUT = 3 * 512*512;

  /* bits needed for address matrix rows */
  parameter ADDR_BITS = $clog2(MUL_SIZE);

  /* memories for matrices */
  reg [DMA_WIDTH-1:0] Amem [0:MUL_SIZE-1];
  reg [DMA_WIDTH-1:0] Bmem [0:MUL_SIZE-1];
  reg [DMA_WIDTH-1:0] outmem [0:MUL_SIZE-1];

  /* control register indicating if operation is running */
  reg running;
  /* start register: will be set to 1 for 1 cycle at the start */
  reg start_mul;

  /* instantiate multiplication vector */
  wire mul_done;
  wire [ADDR_BITS-1:0] a_addr;
  wire [ADDR_BITS-1:0] b_addr;
  wire [ADDR_BITS-1:0] out_addr;
  wire [DMA_WIDTH-1:0] out_data;
  wire out_we;
  matmul_vec#(MUL_SIZE, ADDR_BITS) mul (
    .clk(clk),
    .rst(rst),
    .start_mul(start_mul),
    .mul_done(mul_done),
    .a_addr(a_addr),
    .a_data(Amem[a_addr]),
    .b_addr(b_addr),
    .b_data(Bmem[b_addr]),
    .out_addr(out_addr),
    .out_data(out_data),
    .out_we(out_we)
  );

  /* MMIO read output register */
  reg [MMIO_WIDTH-1:0] mmio_r_result;
  assign mmio_r_data = mmio_r_result;

  /* DMA read output register */
  reg [DMA_WIDTH-1:0] dma_r_result;
  assign dma_r_data = dma_r_result;

  always @ (posedge clk) begin
    if (rst) begin
      running <= 0;
      mmio_r_result <= 0;
      start_mul <= 0;
    end else begin
      /* assert start mul signal for only 1 cycle */
      if (start_mul) begin
        start_mul <= 0;
      end
      /* when block signals its done, clear the running register */
      if (mul_done) begin
        running <= 0;
      end


      /* logic for writing to output memory when out_we is set */
      if (out_we) begin
        outmem[out_addr] <= out_data;
      end


      /* handle MMIO writes */
      if (mmio_w_req) begin
        case (mmio_w_addr)
          /* control register */ 
          'h08: begin
              running <= mmio_w_data[0];
              if (mmio_w_data[0] && !running) begin
                /* start multiplication if not already running */
                start_mul <= 1;
              end
          end
        endcase
      end

      /* handle MMIO reads */
      if (mmio_r_req) begin
        case (mmio_r_addr)
          /* Accelerator size register */
          'h00: mmio_r_result <= MUL_SIZE;
          /* Control/status register */
          'h08: mmio_r_result <= {{(MMIO_WIDTH-1){1'b0}}, {running}};
          /* The next three registers return the (fixed) dma offsets
             for the different buffers */
          'h10: mmio_r_result <= OFF_INA;
          'h18: mmio_r_result <= OFF_INB;
          'h20: mmio_r_result <= OFF_OUT;
        endcase
      end

      /* verilator lint_off WIDTH */
      /* Handle DMA writes */
      if (dma_w_req) begin
        if ((dma_w_addr >= OFF_INA) &&
            (dma_w_addr < (OFF_INA + MUL_SIZE * MUL_SIZE))) begin
          Amem[dma_w_addr / (DMA_WIDTH / 8)] <= dma_w_data;
        end
        if ((dma_w_addr >= OFF_INB) &&
            (dma_w_addr < (OFF_INB + MUL_SIZE * MUL_SIZE))) begin
          Bmem[dma_w_addr / (DMA_WIDTH / 8)] <= dma_w_data;
        end
      end

      /* Handle DMA reads */ 
      if (dma_r_req) begin
        dma_r_result <= outmem[dma_r_addr / (DMA_WIDTH / 8)];
      end
      /* verilator lint_on WIDTH */
    end
  end
endmodule
