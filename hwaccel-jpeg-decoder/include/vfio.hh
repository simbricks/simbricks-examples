#pragma once

#include <cstddef>
#include <fcntl.h>
#include <linux/pci.h>
#include <linux/vfio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define VFIO_GROUP "/dev/vfio/noiommu-0"
#define VFIO_API_VERSION 0

inline int vfio_init(const char *pci_dev);
inline int vfio_map_region(int dev, int idx, void **addr, size_t *len);
inline int vfio_get_region_info(int dev, int idx, struct vfio_region_info *reg);
inline int vfio_busmaster_enable(int dev);

inline int vfio_init(const char *pci_dev) {
  int cont, group, dev;
  struct vfio_group_status g_status = {.argsz = sizeof(g_status)};
  struct vfio_device_info device_info = {.argsz = sizeof(device_info)};

  /* Create vfio container */
  if ((cont = open("/dev/vfio/vfio", O_RDWR)) < 0) {
    fprintf(stderr, "vfio_init: failed to open vfio container.\n");
    return -1;
  }

  /* Check API version of container */
  if (ioctl(cont, VFIO_GET_API_VERSION) != VFIO_API_VERSION) {
    fprintf(stderr, "vfio_init: api version doesn't match.\n");
    goto error_cont;
  }

  if (!ioctl(cont, VFIO_CHECK_EXTENSION, VFIO_NOIOMMU_IOMMU)) {
    fprintf(stderr, "vfio_init: noiommu driver not supported.\n");
    goto error_cont;
  }

  /* Open the vfio group */
  if ((group = open(VFIO_GROUP, O_RDWR)) < 0) {
    fprintf(stderr, "vfio_init: failed to open vfio group.\n");
    goto error_cont;
  }

  /* Test if group is viable and available */
  ioctl(group, VFIO_GROUP_GET_STATUS, &g_status);
  if (!(g_status.flags & VFIO_GROUP_FLAGS_VIABLE)) {
    fprintf(stderr, "vfio_init: group is not viable or avail.\n");
    goto error_group;
  }

  /* Add group to container */
  if (ioctl(group, VFIO_GROUP_SET_CONTAINER, &cont) < 0) {
    fprintf(stderr, "vfio_init: failed to add group to container.\n");
    goto error_group;
  }

  /* Enable desired IOMMU model */
  if (ioctl(cont, VFIO_SET_IOMMU, VFIO_NOIOMMU_IOMMU) < 0) {
    fprintf(stderr, "vfio_init: failed to set IOMMU model.\n");
    goto error_group;
  }

  /* Get file descriptor for device */
  if ((dev = ioctl(group, VFIO_GROUP_GET_DEVICE_FD, pci_dev)) < 0) {
    fprintf(stderr, "vfio_init: failed to get fd for VFIO device.\n");
    goto error_group;
  }

  /* Get device info */
  if (ioctl(dev, VFIO_DEVICE_GET_INFO, &device_info) < 0) {
    fprintf(stderr, "vfio_init: failed to get device info.\n");
    goto error_dev;
  }

  close(group);
  close(cont);
  return dev;

error_dev:
  close(dev);
error_group:
  close(group);
error_cont:
  close(cont);

  return -1;
}

inline int vfio_map_region(int dev, int idx, void **addr, size_t *len) {
  void *ret;
  int prot, flags;
  struct vfio_region_info reg = {.argsz = sizeof(reg)};

  if (vfio_get_region_info(dev, idx, &reg) != 0) {
    fprintf(stderr, "vfio_map_region: failed to get region info.\n");
    return -1;
  }

  prot = PROT_READ | PROT_WRITE;
  flags = MAP_SHARED | MAP_POPULATE;
  ret = mmap(NULL, reg.size, prot, flags, dev, reg.offset);
  if (ret == MAP_FAILED) {
    fprintf(stderr, "vfio_map_region: mmap failed.\n");
    return -1;
  }

  *addr = ret;
  *len = reg.size;

  return 0;
}

inline int vfio_get_region_info(int dev, int idx,
                                struct vfio_region_info *reg) {
  reg->index = idx;

  if (ioctl(dev, VFIO_DEVICE_GET_REGION_INFO, reg)) {
    fprintf(stderr, "vfio_get_region_info: failed to get info.\n");
    return -1;
  }

  return 0;
}

inline int vfio_busmaster_enable(int dev) {
  struct vfio_region_info reg = {.argsz = sizeof(reg)};
  if (vfio_get_region_info(dev, VFIO_PCI_CONFIG_REGION_INDEX, &reg)) {
    fprintf(stderr, "vfio_busmaster_enable: failed to get config region.\n");
    return -1;
  }

  uint16_t cmd_reg = 0;
  if (pread(dev, &cmd_reg, sizeof(cmd_reg), reg.offset + 0x04) !=
      sizeof(cmd_reg)) {
    fprintf(stderr, "vfio_busmaster_enable: failed to read cmd reg.\n");
    return -1;
  }
  cmd_reg |= 0x4; // set bus enable
  if (pwrite(dev, &cmd_reg, sizeof(cmd_reg), reg.offset + 0x04) !=
      sizeof(cmd_reg)) {
    fprintf(stderr, "vfio_busmaster_enable: failed to write cmd reg.\n");
    return -1;
  }

  return 0;
}