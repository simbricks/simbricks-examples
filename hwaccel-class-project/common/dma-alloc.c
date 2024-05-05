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

#include "dma-alloc.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static void *alloc_base = NULL;
static uint64_t alloc_phys_base = 1ULL * 1024 * 1024 * 1024;
static size_t alloc_size = 512 * 1024 * 1024;
static size_t alloc_off = 0;


int dma_alloc_init(void) {
  if (alloc_base) {
    fprintf(stderr, "dma_alloc_init: has already been called\n");
    return -1;
  }

  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0) {
    perror("dma_alloc_init: opening devmem failed");
    return -1;
  }

  void *mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, alloc_phys_base);
  close(fd);
  if (mem == MAP_FAILED) {
    perror("dma_alloc_init: mmap devmem failed");
    return -1;
  }
  alloc_base = mem;
  alloc_off = 0;
  return 0;
}

void *dma_alloc_alloc(size_t size, uintptr_t *paddr)
{
  if (alloc_off + size > alloc_size) {
    fprintf(stderr, "dma_alloc_alloc: no more dma memmory available\n");
    return NULL;
  }

  *paddr = alloc_phys_base + alloc_off;
  void *res = (void *) ((uint64_t) alloc_base + alloc_off);
  alloc_off += size;

  return res;
}
