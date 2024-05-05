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

#include "vfio-pci.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <linux/pci.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>


int vfio_dev_open(struct vfio_dev *dev, const char *groupname,
        const char *pci_dev)
{
    int container, group, device, i;
    struct vfio_group_status group_status =
    { .argsz = sizeof(group_status) };

    /* Create a new container */
    if ((container = open("/dev/vfio/vfio", O_RDWR)) < 0) {
        perror("open vfio failed");
        goto out;
    }

    if (ioctl(container, VFIO_GET_API_VERSION) != VFIO_API_VERSION) {
        perror("unexpected vfio api version");
        goto out_container;
    }

    /* Open the group */
    group = open(groupname, O_RDWR);
    if (group < 0) {
        perror("open group failed");
        goto out_container;
    }

    /* Test the group is viable and available */
    if (ioctl(group, VFIO_GROUP_GET_STATUS, &group_status) < 0) {
        perror("ioctl get status failed");
        goto out_group;
    }

    if (!(group_status.flags & VFIO_GROUP_FLAGS_VIABLE)) {
        fprintf(stderr, "vfio group not viable\n");
        goto out_group;
    }

    /* Add the group to the container */
    if (ioctl(group, VFIO_GROUP_SET_CONTAINER, &container) < 0) {
        fprintf(stderr, "set container failed\n");
        goto out_group;
    }

    /* Enable the IOMMU model we want */
    if (ioctl(container, VFIO_SET_IOMMU, VFIO_NOIOMMU_IOMMU) < 0) {
        fprintf(stderr, "set iommu model failed\n");
        goto out_group;
    }

    /* Get a file descriptor for the device */
    if ((device = ioctl(group, VFIO_GROUP_GET_DEVICE_FD, pci_dev)) < 0) {
        perror("getting device failed");
        goto out_group;
    }

    /* Test and setup the device */
    memset(&dev->info, 0, sizeof(dev->info));
    dev->info.argsz = sizeof(dev->info);
    if ((i = ioctl(device, VFIO_DEVICE_GET_INFO, &dev->info)) < 0) {
        perror("getting device info failed");
        goto out_dev;
    }

    dev->containerfd = container;
    dev->groupfd = group;
    dev->devfd = device;
    return 0;

out_dev:
    close(device);
out_group:
    close(group);
out_container:
    close(container);
out:
    return -1;
}

int vfio_dev_reset(struct vfio_dev *dev)
{
    if (ioctl(dev->devfd, VFIO_DEVICE_RESET) < 0) {
        perror("vfio_dev_reset: reset failed");
        return -1;
    }
    return 0;
}

int vfio_irq_info(struct vfio_dev *dev, uint32_t index,
        struct vfio_irq_info *irq)
{
    memset(irq, 0, sizeof(*irq));

    irq->argsz = sizeof(*irq);
    irq->index = index;

    if (ioctl(dev->devfd, VFIO_DEVICE_GET_IRQ_INFO, irq) < 0) {
        perror("vfio_irq_info: get info failed");
        return -1;
    }

    return 0;
}

int vfio_irq_eventfd(struct vfio_dev *dev, uint32_t index,
        uint32_t count, int *fds)
{
    uint32_t i;
    size_t sz;
    int *pfd;
    void *alloc;
    struct vfio_irq_set *info;

    sz = sizeof(struct vfio_irq_set) + count * sizeof(int);
    if ((alloc = calloc(1, sz)) == NULL) {
        perror("calloc failed");
        return -1;
    }

    pfd = (int *) ((uint8_t *) alloc + sizeof(struct vfio_irq_set));
    for (i = 0; i < count; i++) {
        if ((fds[i] = eventfd(0, EFD_NONBLOCK)) < 0) {
            perror("vfio_irq_eventfd: eventfd failed");
            free(alloc);
            return -1;
        }
        pfd[i] = fds[i];
    }

    info = alloc;
    info->argsz = sz;
    info->flags = VFIO_IRQ_SET_DATA_EVENTFD | VFIO_IRQ_SET_ACTION_TRIGGER;;
    info->index = index;
    info->start = 0;
    info->count = count;

    if (ioctl(dev->devfd, VFIO_DEVICE_SET_IRQS, info) < 0) {
        perror("vfio_irq_eventfd: set failed");
        return -1;
    }

    free(alloc);

    return 0;
}

int vfio_region_info(struct vfio_dev *dev, uint32_t index,
        struct vfio_region_info *reg)
{
    memset(reg, 0, sizeof(*reg));

    reg->argsz = sizeof(*reg);
    reg->index = index;

    if (ioctl(dev->devfd, VFIO_DEVICE_GET_REGION_INFO, reg) < 0) {
        perror("vfio_region_info: get info failed");
        return -1;
    }

    return 0;
}

int vfio_region_map(struct vfio_dev *dev, uint32_t index, void **addr,
        size_t *len)
{
    void *ret;
    struct vfio_region_info reg;

    if (vfio_region_info(dev, index, &reg) != 0) {
        return -1;
    }

    if ((ret = mmap(NULL, reg.size, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_POPULATE, dev->devfd, reg.offset))
            == MAP_FAILED)
    {
        perror("vfio_region_map: mmap failed");
        return -1;
    }

    *addr = ret;
    *len = reg.size;

    return 0;
}

int vfio_read(struct vfio_dev* dev, struct vfio_region_info *reg,
    void* buf, size_t len, uint64_t off)
{
    if(pread(dev->devfd, buf, len, reg->offset + off) != (ssize_t) len)
    {
        perror("vfio_read: pread failed!");
        return -1;
    }
    return 0;
}

int vfio_write(struct vfio_dev* dev, struct vfio_region_info *reg,
    void* buf, size_t len, uint64_t off)
{
    if(pwrite(dev->devfd, buf, len, reg->offset + off) != (ssize_t) len)
    {
        perror("vfio_write: pwrite failed!");
        return -1;
    }
    return 0;
}


int vfio_busmaster_enable(struct vfio_dev* dev)
{
    struct vfio_region_info reg = { .argsz = sizeof(reg) };
    if (vfio_region_info(dev, VFIO_PCI_CONFIG_REGION_INDEX, &reg)) {
        fprintf(stderr, "vfio_busmaster_enable: failed to get config region.\n");
        return -1;
    }

    uint16_t cmd_reg = 0;
    if (pread(dev->devfd, &cmd_reg, sizeof(cmd_reg), reg.offset + 0x04) !=
            sizeof(cmd_reg)) {
        fprintf(stderr, "vfio_busmaster_enable: failed to read cmd reg.\n");
        return -1;
    }
    cmd_reg |= 0x4;  // set bus enable
    if (pwrite(dev->devfd, &cmd_reg, sizeof(cmd_reg), reg.offset + 0x04) !=
            sizeof(cmd_reg)) {
        fprintf(stderr, "vfio_busmaster_enable: failed to write cmd reg.\n");
        return -1;
    }

    return 0;
}