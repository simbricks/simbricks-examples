/* This files contains one suggestion for a simple register-interface to the
   matrix multiply accelerator. The numbers are the offsets (or in one case, bit
   in the register) for the corresponding registers.

   YOU ARE WELCOME TO CHANGE THIS HOWEVER YOU LIKE!
*/

#include <stdint.h>

/**************************************/
/* Register definitions (suggestions)*/

/** Size register: contains supported matrix size (width/height). read-only */
#define REG_SIZE 0x00
/** Register containing size of the on-accelerator memory. read-only */
#define REG_MEM_SIZE 0x08

/** Physical start address of the request queue in host memory. R-W */
#define REG_REQ_BASE 0x10
/** Length of the request queue (# of entries). R-W */
#define REG_REQ_LEN  0x18
/** Index of the next queue entry SW will write to. R-W */
#define REG_REQ_TAIL 0x20

/** Physical start address of the response queue in host memory. R-W */
#define REG_RES_BASE 0x30
/** Length of the request queue (# of entries). R-W */
#define REG_RES_LEN  0x38


/******************************************/
/* Request queue definitions (suggestions)*/

/* Request operation types */
#define REQ_TYPE_DMAREAD  0x1 
#define REQ_TYPE_DMAWRITE 0x2
#define REQ_TYPE_COMPUTE  0x3

/** Flag to request a notification on completion of the request. */
#define REQ_FLAG_NOTIFY 0x1

/** Every entry in the request queue starts with this header.*/
struct req_header {
  /** Request ID specified by SW, can be used to match up completions later. */
  uint32_t req_id;
  /** Request flags (currently only REQ_FLAG_NOTIFY). */
  uint16_t flags;
  /** Request type (see REQ_TYPE_*). */
  uint16_t type;
} __attribute__((packed));

/** Format for a DMA read or write request (same for both). */
struct req_dma {
  /** Request header */
  struct req_header hdr;
  /** Physical address on the host. */
  uint64_t addr;
  /** Offset in accelerator memory. */
  uint64_t mem_off;
  /** Number of bytes to transfer. */
  uint64_t len;
} __attribute__((packed));

/** Format for compute request. */
struct req_compute {
  struct req_header hdr;
  /** Offset of first input matrix in accelerator memory. */
  uint64_t off_ina;
  /** Offset of second input matrix in accelerator memory. */
  uint64_t off_inb;
  /** Offset of output matrix in accelerator memory. */
  uint64_t off_out;
  /** Should output matrix be reset to 0 before start of op? (only 0 or != 0) */
  uint64_t rst_out;
} __attribute__((packed));

/** Union for all request types. */
union req_entry {
  struct req_header hdr;
  struct req_dma dma;
  struct req_compute compute;
  uint8_t raw[64]; /* pad up to cache line size of 64B */
};


/******************************************/
/* Response queue definition (suggestion)*/

struct res_entry {
  /** Request id that has completed (reflected from req). */
  uint32_t req_id;
  /** Request type that has completed. */
  uint32_t type;

  uint8_t _pad[56]; /* pad up to cache line size of 64B */
} __attribute__((packed));