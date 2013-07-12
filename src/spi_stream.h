/*
 * =====================================================================================
 *
 *       Filename:  spi_stream.h
 *
 *    Description:  Abstract a input/output bytestream over a Xilinx
 *                  AXI SPI driver
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#ifndef SPI_STREAM_C98GET05
#define SPI_STREAM_C98GET05

#include "xil_types.h"

struct CircularBuffer;

// Initialize pointers to local I/O stream buffers
void spi_stream_init(CircularBuffer* tx, CircularBuffer* rx);

// Call back function which moves things from data buffer to IO buffers.
// This should be installed in the SPI interrupt routine.
int spi_stream_transfer_data(int error_code);

// Get pointers to the I/O buffers used by the SPI driver
u32* spi_stream_dev_rx_buffer(void);
u32* spi_stream_dev_tx_buffer(void);

#endif /* end of include guard: SPI_STREAM_C98GET05 */
