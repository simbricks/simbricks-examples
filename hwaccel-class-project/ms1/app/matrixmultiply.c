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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <utils.h>

uint8_t *zero_matrix_alloc(size_t m, size_t n) {
  return calloc(m * n, sizeof(uint8_t));
}

uint8_t *rand_matrix_alloc(size_t m, size_t n) {
  uint8_t *A = zero_matrix_alloc(m, n);
  size_t i;
  for (i = 0; i < m * n; i++)
    A[i] = rand();
  return A;
}

void matmult(const uint8_t *inA, const uint8_t *inB, uint8_t *out, size_t n) {
  size_t i, j, k;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      out[i * n + j] = 0;
      for (k = 0; k < n; k++) {
        out[i * n + j] += inA[i * n + k] * inB[k * n + j];
      }
    }
  }
}

void measure(size_t n, size_t iterations)
{
  uint8_t *A = rand_matrix_alloc(n, n);
  uint8_t *B = rand_matrix_alloc(n, n);
  uint8_t *out = zero_matrix_alloc(n, n);

  size_t i;
  uint64_t total_cycles = 0;
  for (i = 0; i < iterations; i++) {
    uint64_t start = rdtsc();
    matmult(A, B, out, n);
    total_cycles += rdtsc() - start;
  }

  printf("Cycles per operation: %ld\n", total_cycles / iterations);
}

int main(int argc, char *argv[])
{
  measure(256, 10);
  return 0;
}
