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

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <vfio-pci.h>
#include <dma-alloc.h>

#include "../common/reg_defs.h"
#include "driver.h"

//#define DEBUG
#ifdef DEBUG
#define dprintf(x...) fprintf(stderr, "DRV: "x)
#else
#define dprintf(...) do { } while (0)
#endif

/** Use this macro to safely access a register at a specific offset */
#define ACCESS_REG(r) (*(volatile uint64_t *) ((uintptr_t) regs + r))

static void *regs;

static size_t dma_mem_size = 16 * 1024 * 1024;
static uint8_t *dma_mem;
static uintptr_t dma_mem_phys;

static uint8_t *dma_mem_w;
static uintptr_t dma_mem_phys_w;

size_t matrix_size = -1ULL;
size_t off_ina;
size_t off_inb;
size_t off_out;

int accelerator_init(bool dma) {
  struct vfio_dev dev;
  size_t reg_len;

  if (vfio_dev_open(&dev, "/dev/vfio/noiommu-0", "0000:00:00.0") != 0) {
    fprintf(stderr, "open device failed\n");
    return -1;
  }

  if(vfio_region_map(&dev, 0, &regs, &reg_len)) {
    fprintf(stderr, "mapping registers failed\n");
    return -1;
  }

  if (dma_alloc_init()) {
    fprintf(stderr, "DMA INIT failed\n");
    return -1;
  }

  if (!(dma_mem = dma_alloc_alloc(dma_mem_size, &dma_mem_phys))) {
    fprintf(stderr, "Allocating DMA memory failed\n");
    return -1;
  }

  dma_mem_w = (uint8_t *) dma_mem + (dma_mem_size / 2);
  dma_mem_phys_w = dma_mem_phys + (dma_mem_size / 2);

  if (vfio_busmaster_enable(&dev)) {
    fprintf(stderr, "Enabling busmastering failed\n");
    return -1;
  }


  matrix_size = ACCESS_REG(REG_SIZE);
  off_ina = ACCESS_REG(REG_OFF_INA);
  off_inb = ACCESS_REG(REG_OFF_INB);
  off_out = ACCESS_REG(REG_OFF_OUT);

  return 0;
}

int accelerator_matrix_size(void) {
  return matrix_size;
}

static inline void put_accel_in(uint64_t dst_off,
                                const uint8_t *in,
                                size_t cols,
                                size_t rows,
                                size_t in_rowlen,
                                bool transpose) {
  size_t i, j;

  dprintf("DMA-ing data onto accelerator\n");
  /** copy into dma-able region */
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      uint8_t x = in[i * in_rowlen + j];
      if (!transpose)
        dma_mem[i * cols + j] = x;
      else
        dma_mem[j * cols + i] = x;
    }
  }
  ACCESS_REG(REG_DMA_LEN) = cols * rows;
  ACCESS_REG(REG_DMA_OFF) = dst_off;
  ACCESS_REG(REG_DMA_ADDR) = dma_mem_phys;

  ACCESS_REG(REG_DMA_CTRL) = REG_DMA_CTRL_RUN;
  while ((ACCESS_REG(REG_DMA_CTRL) & REG_DMA_CTRL_RUN));
  dprintf("DMA'd data onto accelerator\n");
}

static inline void get_accel_out(uint8_t *out,
                                 uint64_t src_off,
                                 size_t n,
                                 size_t out_rowlen) {
  size_t i, j;
  dprintf("DMA-ing data out of accelerator\n");
  ACCESS_REG(REG_DMA_LEN) = n * n;
  ACCESS_REG(REG_DMA_OFF) = src_off;
  ACCESS_REG(REG_DMA_ADDR) = dma_mem_phys_w;

  ACCESS_REG(REG_DMA_CTRL) = REG_DMA_CTRL_RUN | REG_DMA_CTRL_W;
  while ((ACCESS_REG(REG_DMA_CTRL) & REG_DMA_CTRL_RUN));

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j += 8)
      *(uint64_t *) (out + (i * out_rowlen + j)) +=
        *(uint64_t *) &dma_mem_w[i * n + j];
  dprintf("DMA'd data out of accelerator\n");
}

static inline void exec_accel()
{
  dprintf("Executing on accelerator\n");
  ACCESS_REG(REG_CTRL) = REG_CTRL_RUN;

  while ((ACCESS_REG(REG_CTRL) & REG_CTRL_RUN));
  dprintf("Executed on accelerator\n");
}

static void dump_matrix(const uint8_t *m, size_t n)
{
#ifdef DEBUG
  size_t i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      printf("%02x ", m[i * n + j]);
    }
    printf("\n");
  }
#endif
}

void matmult_accel(const uint8_t * restrict A, const uint8_t * restrict B,
                   uint8_t * restrict out, size_t n) {
  if ((int) n < accelerator_matrix_size()) {
    fprintf(stderr, "matmult_accel: matrix smaller than supported\n");
    abort();
  }

  size_t block = matrix_size;
  size_t i, j, k;
  assert(n % block == 0);
  size_t n_b = n / block;

  dprintf("\nInput A:\n");
  dump_matrix(A, n);
  dprintf("\nInput B:\n");
  dump_matrix(B, n);

  memset(out, 0, n * n);
  for (i = 0; i < n_b; i++) {
    for (j = 0; j < n_b; j++) {
      for (k = 0; k < n_b; k++) {
        put_accel_in(off_ina, A + ((i * block * n) + k * block), block, block, n, false);
        put_accel_in(off_inb, B + ((k * block * n) + j * block), block, block, n, true);
        exec_accel();
        get_accel_out(out + ((i * block * n) + j * block), off_out, block, n);
      }
      
    }
  }

  dprintf("Output:\n");
  dump_matrix(out, n);
}
