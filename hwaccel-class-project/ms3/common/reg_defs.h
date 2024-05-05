/* This files contains one suggestion for a simple register-interface to the
   matrix multiply accelerator. The numbers are the offsets (or in one case, bit
   in the register) for the corresponding registers.

   YOU ARE WELCOME TO CHANGE THIS HOWEVER YOU LIKE!
*/

/** Size register: contains supported matrix size (width/height). read-only */
#define REG_SIZE 0x00

/** Control register: used to start the accelerator/wait for completion.
    read/write */
#define REG_CTRL 0x08
/** Run bit in the control register, set to start, then wait for device to clear
    this back to 0.*/
#define REG_CTRL_RUN 0x01
/** Reset bit for output buffer. When set, clears the output buffer to 0. */
#define REG_CTRL_RSTOUT 0x02

/** Registers specifying offset for input and output matrices in the accelerator
 * memory. now read-writeable. */
#define REG_OFF_INA 0x10
#define REG_OFF_INB 0x18
#define REG_OFF_OUT 0x20

/** Register containing size of the on-accelerator memory. read-only */
#define REG_MEM_SIZE 0x30
/** Register containing offset of the accelerator memory. read-only */
#define REG_MEM_OFF 0x38


/** DMA Control register. */
#define REG_DMA_CTRL 0x40
/** Bit to request start of DMA operation */
#define REG_DMA_CTRL_RUN 0x1
/** Bit to request DMA Write (device to host), if 0 the operation is a read
 * instead (host to device). */
#define REG_DMA_CTRL_W   0x2

/** Register holding requested length of the DMA operation in bytes */
#define REG_DMA_LEN  0x48
/** Register holding *physical* address on the host. */
#define REG_DMA_ADDR 0x50
/** Register holding memory offset on the accelerator. */
#define REG_DMA_OFF  0x58
