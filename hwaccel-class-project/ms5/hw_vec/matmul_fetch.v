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
 * This module implements the fetch state of the matmul. Fetches row vectors
 * from the left matrix and column vectors of the second matrix output cell by
 * output cell. Iterates through columns and then rows.
 *
 * Accesses A and B memories through a/b_addr/data. Note that the data value
 * appears in the cycle after addr has been set.
 *
 * We then store the results in registers, and output them in the following
 * cycle to row/col(_no). Valid is asserted to 1 whenever the output row/col
 * hold valid data.
 *
 * This is effectively a 2 stage pipeline. One stage generates addresses to
 * fetch, and the second one takes the outputs and passes them on a cycle later.
 */
module matmul_fetch #(parameter
  MUL_SIZE = 8,
  ADDR_BITS = $clog2(MUL_SIZE)
) (
  input  clk,
  input  rst,

  /* Memory Hookups */
  output [ADDR_BITS - 1:0] a_addr,
  input  [MUL_SIZE * 8 - 1:0] a_data,
  output [ADDR_BITS - 1:0] b_addr,   /* remember B memory is transposed */
  input [MUL_SIZE * 8 - 1:0] b_data, 

  /* Handshake: request to start an operation */
  input start,
  /* Handshake: output row and column along with *_no are valid */
  output valid,

  /* Output row and column */
  output [ADDR_BITS - 1:0] row_no,
  output [8 * MUL_SIZE - 1:0] row,
  output [ADDR_BITS - 1:0] col_no,
  output [8 * MUL_SIZE - 1:0] col
);

  parameter last_idx = {ADDR_BITS{1'b1}};

  /* track if we are actively fetching (internal) */
  reg fetching;

  /* register that outputs whether something was fetched (fetching delayed by
     one cycle) */
  reg fetched;
  assign valid = fetched;

  /* hold addresses to fetch */
  reg [ADDR_BITS - 1:0] row_fetch;
  reg [ADDR_BITS - 1:0] col_fetch;
  assign a_addr = row_fetch;
  assign b_addr = col_fetch;

  /* register for fetched row and column as well as numbers thereof */
  reg [ADDR_BITS - 1:0] row_fetched;
  reg [ADDR_BITS - 1:0] col_fetched;
  reg [8 * MUL_SIZE - 1:0] row_vec;
  reg [8 * MUL_SIZE - 1:0] col_vec;
  assign row_no = row_fetched;
  assign row = row_vec;
  assign col_no = col_fetched;
  assign col = col_vec;

  /* state machine */
  always @ (posedge clk) begin
    if (rst) begin
      /* FILL ME IN */
    end else begin
      /* start off fetching if it was not running and start requested */
      if (!fetching && start) begin
        /* FILL ME IN */
      end

      /* fetching state machine */
      if (fetching) begin
        /* FILL ME IN */
      end
    end
  end
endmodule

