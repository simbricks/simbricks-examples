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

#ifndef VFIO_PCI_H_
#define VFIO_PCI_H_

#include <linux/vfio.h>
#include <stddef.h>
#include <stdint.h>

struct vfio_dev {
    int containerfd;
    int groupfd;
    int devfd;

    struct vfio_device_info info;
};


int vfio_dev_open(struct vfio_dev *dev, const char *groupname,
                  const char *pci_dev);

int vfio_dev_reset(struct vfio_dev *dev);

int vfio_irq_info(struct vfio_dev *dev, uint32_t index,
                  struct vfio_irq_info *irq);

int vfio_irq_eventfd(struct vfio_dev *dev, uint32_t index, uint32_t count,
                     int *fds);

int vfio_region_info(struct vfio_dev *dev, uint32_t index,
                     struct vfio_region_info *reg);

int vfio_region_map(struct vfio_dev *dev, uint32_t index,
                    void **addr, size_t *len);

int vfio_read(struct vfio_dev* dev, struct vfio_region_info *reg,
    void* buf, size_t len, uint64_t off);

int vfio_write(struct vfio_dev* dev, struct vfio_region_info *reg,
    void* buf, size_t len, uint64_t off);

int vfio_busmaster_enable(struct vfio_dev* dev);

#define READ_REG32(b, o) *((volatile uint32_t *) (((uint8_t *) b) + o))
#define WRITE_REG32(b, o, d) (*((volatile uint32_t *)((uint8_t *) b + o)) = ((uint32_t) d))

#define READ_REG64(b, o) *((volatile uint64_t *) (((uint8_t *) b) + o))
#define WRITE_REG64(b, o, d) (*((volatile uint64_t *)((uint8_t *) b + o)) = ((uint64_t) d))

#endif /* ndef VFIO_PCI_H_ */