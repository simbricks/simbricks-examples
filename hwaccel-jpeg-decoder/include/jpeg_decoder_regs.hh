#pragma once
#include <cstdint>

#define CTRL_REG_START_BIT 0x80000000
#define CTRL_REG_LEN_MASK 0x00FFFFFF

// control registers for the JPEG decoder, based on
// https://github.com/ultraembedded/core_jpeg_decoder
struct __attribute__((packed)) JpegDecoderRegs {
  uint32_t ctrl;
  uint32_t isBusy;
  uint32_t src;
  uint32_t dst;
};

struct __attribute__((packed)) VerilatorRegs {
  // activates or deactivates tracing
  bool tracing_active;
};
