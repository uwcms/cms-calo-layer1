#include "spi_stream.h"

#include <stdlib.h>
#include <stdio.h>

#include "circular_buffer.h"
#include "protocol.h"

SPIStream* spi_stream_init(CircularBuffer* tx, CircularBuffer* rx,
    void (*transmit_callback)(u8*, u8*, u16)) {
  SPIStream* stream = malloc(sizeof(SPIStream));
  stream->tx_buffer = tx;
  stream->rx_buffer = rx;
  stream->waiting_for_packet_id = 0;
  stream->transmit_callback = transmit_callback;
  spi_stream_load_tx(stream, 0);
  return stream;
}

void spi_stream_load_tx(SPIStream* stream, u32 pkt_id) {
  spi_stream_construct_tx_packet(
      pkt_id,
      stream->spi_tx_buffer[pkt_id % SPI_BUFFER_TX_HISTORY],
      SPI_BUFFER_SIZE,
      stream->tx_buffer);
}

void spi_stream_transfer_data(SPIStream* stream, int error) {
  // verify the incoming data.
  int cksum_error = 0;
  u32 received_packet_id = spi_stream_verify_packet(
      stream->spi_rx_buffer, SPI_BUFFER_SIZE, &cksum_error);

//  printf("transfer start: (ck)%li (rec)%li (want)%li\n", 
//      (u32)cksum_error, received_packet_id, stream->waiting_for_packet_id);

  // if the data is corrupt: resend the previous packet
  u32 pkt_to_send = stream->waiting_for_packet_id;

  // check if we got what we sent last go round
  if (!cksum_error && !error) {
    if (received_packet_id == stream->waiting_for_packet_id) {
      // read the data into the RX buffer
      int read_ok = spi_stream_read_rx_packet(
          stream->spi_rx_buffer, stream->rx_buffer);
      if (read_ok) {
        // advance to next packet, load the data.
        stream->waiting_for_packet_id++;
        spi_stream_load_tx(stream, stream->waiting_for_packet_id);
        pkt_to_send = stream->waiting_for_packet_id;
      }
    } else {
      // The remote send an older packet than we were expecting - that means
      // it wants us to rewind to that packet
      pkt_to_send = received_packet_id;
    }
  }

//  printf("transfer end: (send)%li (want)%li\n", 
//      pkt_to_send, stream->waiting_for_packet_id);

  stream->transmit_callback(
      (void*)(stream->spi_tx_buffer[pkt_to_send % SPI_BUFFER_TX_HISTORY]),
      (void*)stream->spi_rx_buffer,
      SPI_BUFFER_SIZE * sizeof(u32));
}
