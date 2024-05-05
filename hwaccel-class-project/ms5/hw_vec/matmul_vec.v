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

module matmul_vec #(parameter
  MUL_SIZE = 8,
  ADDR_BITS = $clog2(MUL_SIZE)
) (
  input  clk,
  input  rst,

  /* start request: only asserted for one cycle */
  input start_mul,
  /* multiplication is done, only asserted for one cycle*/
  output mul_done,

  /* hookups to input and output memories */
  output [ADDR_BITS - 1:0] a_addr,
  input  [MUL_SIZE * 8 - 1:0] a_data,
  output [ADDR_BITS - 1:0] b_addr,
  input [MUL_SIZE * 8 - 1:0] b_data,
  output [ADDR_BITS - 1:0] out_addr,
  output [MUL_SIZE * 8 - 1:0] out_data,
  output out_we
);

  parameter last_idx = {ADDR_BITS{1'b1}};

  /* first stage: fetch vectors from A and B memories in order needed */
  wire fetched;
  wire [ADDR_BITS - 1:0] f_row_no;
  wire [8 * MUL_SIZE - 1:0] f_row;
  wire [ADDR_BITS - 1:0] f_col_no;
  wire [8 * MUL_SIZE - 1:0] f_col;
  matmul_fetch #(MUL_SIZE, ADDR_BITS) fetch (
    .clk(clk),
    .rst(rst),

    .a_addr(a_addr),
    .a_data(a_data),
    .b_addr(b_addr),
    .b_data(b_data),
    .start(start_mul),
    .valid(fetched),
    .row_no(f_row_no),
    .row(f_row),
    .col_no(f_col_no),
    .col(f_col)
  );

  /* second stage: calculate dot product, assemble outptu rows */
  wire c_ready;
  wire [ADDR_BITS - 1:0] c_row_no;
  wire [8 * MUL_SIZE - 1:0] c_row;
  matmul_calc #(MUL_SIZE, ADDR_BITS) calc (
    .clk(clk),
    .rst(rst),

    .in_valid(fetched),
    .in_row_no(f_row_no),
    .in_row(f_row),
    .in_col_no(f_col_no),
    .in_col(f_col),

    .out_ready(c_ready),
    .out_row_no(c_row_no),
    .out_row(c_row)
  );
  assign out_addr = c_row_no;
  assign out_data = c_row;
  assign out_we = c_ready;

  assign mul_done = (c_row_no == last_idx) && c_ready;
endmodule
