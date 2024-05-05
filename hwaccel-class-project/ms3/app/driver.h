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

#ifndef DRIVER_H_
#define DRIVER_H_

#include <stdbool.h>

/* YOU SHOULY KEEP THIS INTERFACE AS IS */

/**
 * Initialize the accelerator. Must only be called once. Returns 0 on success,
 * and != 0 on failure.
 */
int accelerator_init(bool dma);

/**
 * Return supported matrix size of matrix-multiply accelerator. Must only be
 * called after a successful call to accelerator_init.
 */
int accelerator_matrix_size(void);

/**
 * Run a square matrix multiplication through the accelerator. Must only be
 * called after a successful call to accelerator_init.
 *
 * @param A   Left input matrix
 * @param B   Right input matrix
 * @param out Output Matrix
 * @param n   Matrix size (width/height for A, B, out)
 */
void matmult_accel(const uint8_t *restrict A, const uint8_t *restrict B,
                   uint8_t *restrict out, size_t n);

// Potenitally useful helper
uint8_t *zero_matrix_alloc(size_t m, size_t n);

#endif  // ndef DRIVER_H_

