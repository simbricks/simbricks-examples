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

#ifndef ACCEL_SIM_SIM_H_
#define ACCEL_SIM_SIM_H_

#include <stdint.h>

#include <simbricks/pcie/proto.h>

/******************************************************************************/
/* Utility definitions provided by the framework. */

/** Current simulation time (in piocoseconds) */
extern uint64_t main_time;

/** Allocate a PCIe message to send out. Must be followed by a call to
 *  SendPcieOut. */
volatile union SimbricksProtoPcieD2H *AllocPcieOut(void);

/** Send out a PCIe message. msg must have been previously allocated with
 * AllocPcieOut.
 * @param type SimBricks PCIe message type.
 */
void SendPcieOut(volatile union SimbricksProtoPcieD2H *msg, uint64_t type);

/** Parameter initialized to desired operation latency (in picoseconds) */
extern uint64_t op_latency;
/** Parameter initialized to desired matrix size for the accelerator
 * (width/height) */
extern uint64_t matrix_size;


/******************************************************************************/
/* Functions you will implement in sim.c */

/**
 * Called once during initialization. Use this to initialize your internal
 * simulation model state.
*/
int InitState(void);

/**
 * Called for every register read. You must complete each operation with a
 * corresponding read completion sent with SendPcieOut.
*/
void MMIORead(volatile struct SimbricksProtoPcieH2DRead *read);

/**
 * Called for every register write. No completion message needed.
*/
void MMIOWrite(volatile struct SimbricksProtoPcieH2DWrite *write);

/** Called by simulation loop to enable you to process potentially pending
 * events in your simulation model. */
void PollEvent(void);

/** Must return the time your next simulation event will be due to be processed
 * by PollEvent (or an earlier time). If no event is pending, return
 * UINT64_MAX. */
uint64_t NextEvent(void);

#endif  // ndef ACCEL_SIM_SIM_H_