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

// Size of the buffer we pass back and forth to the SPI device
#define SPI_BUFFER_SIZE 512 // words
#define SPI_BUFFER_TX_HISTORY 4

typedef struct {
  u32 spi_rx_buffer[SPI_BUFFER_SIZE];
  // we maintain a history of the transmitted data.
  u32 spi_tx_buffer[SPI_BUFFER_TX_HISTORY][SPI_BUFFER_SIZE];
  u32 waiting_for_packet_id;
  CircularBuffer* tx_buffer;
  CircularBuffer* rx_buffer;
  void (*transmit_callback)(u8* tx, u8* rx, u16 nbytes);
} SPIStream;

// Initialize pointers to local I/O stream buffers
SPIStream* spi_stream_init(CircularBuffer* tx, CircularBuffer* rx);

// Load data into the output buffer, corresponding to pkt_id
void spi_stream_load_tx(SPIStream* stream, u32 pkt_id);

// Call back function which moves things from data buffer to IO buffers.
// This should be installed in the SPI interrupt routine.
void spi_stream_transfer_data(SPIStream* stream, int error_code);

#endif /* end of include guard: SPI_STREAM_C98GET05 */
