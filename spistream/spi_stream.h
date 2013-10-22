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

#include <stdint.h>
#include "circular_buffer.h"

// Size of the buffer we pass back and forth to the SPI device
#define SPI_BUFFER_SIZE 512 // words
#define SPI_BUFFER_TX_HISTORY 2

typedef struct {
  uint32_t spi_rx_buffer[SPI_BUFFER_SIZE];
  // we maintain a history of the transmitted data.
  uint32_t spi_tx_buffer[SPI_BUFFER_TX_HISTORY][SPI_BUFFER_SIZE];
  uint32_t waiting_for_packet_id;
  uint32_t on_packet_id;
  CircularBuffer* tx_buffer;
  CircularBuffer* rx_buffer;
  void (*transmit_callback)(uint8_t* tx, uint8_t* rx, uint16_t nbytes);
} SPIStream;

// Initialize pointers to local I/O stream buffers, and provide a callback
// which should tell the device driver to begin the transfer.  The initial
// packet ID must match between the master and the slave (advise using zero).
SPIStream* spi_stream_init(
    CircularBuffer* tx, CircularBuffer* rx,
    void (*transmit_callback)(uint8_t*, uint8_t*, uint16_t),
    uint32_t initial_packet_id);

// Return the value of a - b, wrapping at the edges.
int delta_packet_id(uint32_t a, uint32_t b);

// Load data into the output buffer, corresponding to pkt_id
void spi_stream_load_tx(SPIStream* stream, uint32_t pkt_id);

// Call back function which moves things from data buffer to IO buffers.
// This should be installed in the SPI interrupt routine.
void spi_stream_transfer_data(SPIStream* stream, int error_code);

// free memory associated w/ a SPIStream
void spi_stream_free(SPIStream* stream);

#endif /* end of include guard: SPI_STREAM_C98GET05 */
