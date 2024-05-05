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

/**
 * Multiplication calculation stage. Takes in a stream of input row/column pairs
 * and outputs a stream of output rows (one output row for every N input pairs).
 */
module matmul_calc #(parameter
  MUL_SIZE = 8,
  ADDR_BITS = $clog2(MUL_SIZE)
) (
  input  clk,
  input  rst,

  /* handhshake: input row and col are valid */
  input in_valid,
  /* handshake: outptu row and no is valid. only asserted every N cycle when the
    row is completed */
  output out_ready,

  /* Input row and column along with numbers */
  input [ADDR_BITS - 1:0] in_row_no,
  input [8 * MUL_SIZE - 1:0] in_row,
  input [ADDR_BITS - 1:0] in_col_no,
  input [8 * MUL_SIZE - 1:0] in_col,

  /* complete output row vector for the matrix along with the row number */
  output [ADDR_BITS - 1:0] out_row_no,
  output [8 * MUL_SIZE - 1:0] out_row
);

  parameter last_idx = {ADDR_BITS{1'b1}};

  /* out_el <= combinatorial dot product of in_row and in_col */
  reg [7:0] out_el;
  integer i;
  always @(*)
  begin
    /* FILL ME IN */
  end

  /* combinatorial re-formatting of out_arr into bitvector */
  reg [7:0] out_arr [0:MUL_SIZE-1];
  reg [8 * MUL_SIZE - 1:0] out_vec;
  integer j;
  always @(*)
  begin
    for (j = 0; j < MUL_SIZE; j = j + 1) begin
      out_vec[j * 8 +: 8] = out_arr[j];
    end
  end

  /* store and delay output row */
  reg [ADDR_BITS - 1:0] out_no;
  /* handshake register: indicating when an output row is ready */
  reg ready;

  assign out_row_no = out_no;
  assign out_row  = out_vec;
  assign out_ready = ready;

  always @ (posedge clk) begin
    if (rst) begin
      /* FILL ME IN */
    end else begin
      /* calculating state machine */
      if (in_valid) begin
        /* FILL ME IN */
      end else begin
        /* FILL ME IN */
      end
    end
  end
endmodule

