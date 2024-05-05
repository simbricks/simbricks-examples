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

/* Combinatorial circuit that takes full matrices as bit vectors and produces
   an output bit vector continuously. */
module matmul_comb #(parameter
  MUL_SIZE = 8
) (
  input  [8 * MUL_SIZE * MUL_SIZE - 1:0]  a,
  input  [8 * MUL_SIZE * MUL_SIZE - 1:0]  b,
  output [8 * MUL_SIZE * MUL_SIZE - 1:0]  out
);

  /* this is not actually a register, but need reg to be able to write to
     different slices from a loop. Verilog only allows this through a
     pseudo-register and from an always block */
  reg [8 * MUL_SIZE * MUL_SIZE - 1:0] tmp;
  integer i, j, k;

  /* wire tmp up to the out port */
  assign out = tmp;

  /* fully combinatorial */
  always @(*)
  begin
    /* remember that these loops get fully unrolled */
    for (i = 0; i < MUL_SIZE; i = i + 1) begin
      for (j = 0; j < MUL_SIZE; j = j + 1) begin
        tmp[(i * MUL_SIZE + j) * 8 +: 8] = 0;
        for (k = 0; k < MUL_SIZE; k = k + 1) begin
          tmp[(i * MUL_SIZE + j) * 8 +: 8] =
            tmp[(i * MUL_SIZE + j) * 8 +: 8] +
            (a[(i * MUL_SIZE + k) * 8 +: 8] * b[(k * MUL_SIZE + j) * 8 +: 8]);
        end
      end
    end
  end
endmodule
