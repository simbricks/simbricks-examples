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

/* This is the top-level module of the design. Think of the inputs and outputs
   on this as the pins on the chip. */
module top #(parameter
    /** MMIO data width */
    MMIO_WIDTH = 32,
    /** MMIO address width */
    MMIO_ADDRBITS = 32,
    /** DMA data width */
    DMA_WIDTH = 256,
    /** DMA address width */
    DMA_ADDRBITS = 32,
    /* Actual multiplication matrix size: MUL_SIZE x MUL_SIZE */
    MUL_SIZE = 8
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

  /* dma offsets for the matric memories */
  parameter OFF_INA = 512*512;
  parameter OFF_INB = 2 * 512*512;
  parameter OFF_OUT = 3 * 512*512;

  /* figure out how many `DMA_WIDTH`-wide entries we need for the full
     matrix. */
  parameter MEMSZ = (MUL_SIZE * MUL_SIZE * 8) / DMA_WIDTH;

  /* memories for input and output matrices */
  reg [DMA_WIDTH-1:0] Amem [0:MEMSZ-1];
  reg [DMA_WIDTH-1:0] Bmem [0:MEMSZ-1];
  reg [DMA_WIDTH-1:0] outmem [0:MEMSZ-1];

  /* output from the multiply unit*/
  wire [MUL_SIZE * MUL_SIZE * 8 - 1:0] out;

  /* reformat a / b into matrix, outmemNew from matrix */
  /* (this is purely combinatorial reformatting of Amem,Bmem) */
  reg [MUL_SIZE * MUL_SIZE * 8 - 1:0] a;
  reg [MUL_SIZE * MUL_SIZE * 8 - 1:0] b;
  integer i, j, k, l;
  always @ (*) begin
    for (i = 0; i < MUL_SIZE; i = i + 1) begin
      for (j = 0; j < MUL_SIZE; j = j + 1) begin
        k = ((i * MUL_SIZE) + j);
        l = k % (DMA_WIDTH / 8);
        a[(i * MUL_SIZE + j) * 8 +: 8] = Amem[k / (DMA_WIDTH / 8)][l * 8 +: 8];
        // Note the transposition for B
        b[(j * MUL_SIZE + i) * 8 +: 8] = Bmem[k / (DMA_WIDTH / 8)][l * 8 +: 8];
      end
    end
  end

  /* instantiate combinatorial matrix multiplier, and wire up inputs and
     outputs */
  matmul_comb#(MUL_SIZE) mul (
    .a(a),
    .b(b),
    .out(out)
  );

  /* output register for mmio result. Holds value after mmio_r_req is zero */
  reg [MMIO_WIDTH-1:0] mmio_r_result;
  assign mmio_r_data = mmio_r_result;

  /* DMA read output register */
  reg [DMA_WIDTH-1:0] dma_r_result;
  assign dma_r_data = dma_r_result;

  /* control register indicating if operation is currently running */
  reg running;

  /* actual state machine: evaluated at every rising clock edge */
  always @ (posedge clk) begin
    if (rst) begin
      /* reset logic, needs to set all important state to a sane initial
         state. */
      running <= 0;
      mmio_r_result <= 0;
    end else begin
      /* state machine to keep accelerator running for exactly one cycle before
         writing output to memory and clearing running bit */
      if (running) begin
        integer m;
        for (m = 0; m < MEMSZ; m = m + 1) begin
          outmem[m] <= out[m * DMA_WIDTH +: DMA_WIDTH];
        end
        running <= 0;
      end

      /* Handle mmio writes */
      if (mmio_w_req) begin
        case (mmio_w_addr)
          /* only the control register is writable */
          'h08: running <= mmio_w_data[0];
        endcase
      end

      /* Handle mmio reads */
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


      /* temporarily disable width mismatch warning to avoid truncating dma
         addresses manually. It's ugly enough as it is... */
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
