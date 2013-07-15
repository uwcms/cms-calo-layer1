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
#define SPI_STREAM_STATE_NORMAL 0x1
#define SPI_STREAM_STATE_WAITING 0x2
#define SPI_STREAM_STATE_RESEND 0x3
#define SPI_STREAM_STATE_RESEND_WAIT_ACK 0x4

typedef struct {
  u32 spi_rx_buffer[SPI_BUFFER_SIZE];
  u32 spi_tx_buffer_a[SPI_BUFFER_SIZE];
  u32 spi_tx_buffer_b[SPI_BUFFER_SIZE];
  u32* spi_tx_buffer_prev;
  u32* spi_tx_buffer_current;
  CircularBuffer* tx_buffer;
  CircularBuffer* rx_buffer;
  u32 waiting_for_packet_id;
  u32 stream_state;
} SPIStream;

// Initialize pointers to local I/O stream buffers
void spi_stream_init(SPIStream* stream, CircularBuffer* tx, CircularBuffer* rx) {

// Call back function which moves things from data buffer to IO buffers.
// This should be installed in the SPI interrupt routine.
int spi_stream_transfer_data(int error_code);

// Get pointers to the I/O buffers used by the SPI driver
u32* spi_stream_dev_rx_buffer(void);
u32* spi_stream_dev_tx_buffer(void);

#endif /* end of include guard: SPI_STREAM_C98GET05 */
