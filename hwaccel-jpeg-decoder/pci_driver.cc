#include <fcntl.h>
#include <linux/vfio.h>
#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

#include "include/jpeg_decoder_regs.hh"
#include "include/vfio.hh"

int main(int argc, char *argv[]) {
  if (argc != 6) {
    std::cerr << "usage: pci_driver pci-device dma_src dma_src_len "
                 "dma_dest produce_waveform\n";
    return EXIT_FAILURE;
  }
  bool produce_waveform = std::stoi(argv[5]);

  int vfio_fd = vfio_init(argv[1]);
  if (vfio_fd < 0) {
    std::cerr << "vfio init failed" << std::endl;
    return 1;
  }

  // map the BARs
  void *bar0;
  size_t reg_len;
  if (vfio_map_region(vfio_fd, 0, &bar0, &reg_len)) {
    std::cerr << "vfio_map_region for bar 0 failed" << std::endl;
    return 1;
  }
  void *bar1;
  if (vfio_map_region(vfio_fd, 1, &bar1, &reg_len)) {
    std::cerr << "vfio_map_region for bar 1 failed" << std::endl;
    return 1;
  }

  // required for DMA
  if (vfio_busmaster_enable(vfio_fd)) {
    std::cerr << "vfio busmiaster enable faled" << std::endl;
    return 1;
  }

  volatile JpegDecoderRegs &jpeg_decoder_regs =
      *static_cast<volatile JpegDecoderRegs *>(bar0);
  volatile VerilatorRegs &verilator_regs =
      *static_cast<volatile VerilatorRegs *>(bar1);

  if (jpeg_decoder_regs.isBusy) {
    std::cerr << "error: jpeg decoder is unexpectedly busy\n";
    return 1;
  }

  uintptr_t dma_src_addr = std::stoul(argv[2], nullptr, 0);
  uint32_t dma_src_len = std::stoul(argv[3], nullptr, 0);
  uintptr_t dma_dst_addr = std::stoul(argv[4], nullptr, 0);

  // submit image to decode
  std::cout << "info: submitting image to jpeg decoder\n";
  if (produce_waveform) {
    verilator_regs.tracing_active = true;
  }
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  jpeg_decoder_regs.src = dma_src_addr;
  jpeg_decoder_regs.dst = dma_dst_addr;

  // invoke accelerator
  jpeg_decoder_regs.ctrl = dma_src_len | CTRL_REG_START_BIT;

  // wait until decoding finished
  while (jpeg_decoder_regs.isBusy) {
  }

  // report duration
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  if (produce_waveform) {
    verilator_regs.tracing_active = false;
  }

  std::cout << "duration: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
                   .count()
            << " ns\n";
  return 0;
}