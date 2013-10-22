#include "spi_stream.h"

#include <stdlib.h>

#include <stdio.h>
#include <inttypes.h>

#include "circular_buffer.h"
#include "protocol.h"

SPIStream* spi_stream_init(CircularBuffer* tx, CircularBuffer* rx,
    void (*transmit_callback)(uint8_t*, uint8_t*, uint16_t), uint32_t initial_packet_id) {
  SPIStream* stream = (SPIStream*)malloc(sizeof(SPIStream));
  stream->tx_buffer = tx;
  stream->rx_buffer = rx;
  stream->waiting_for_packet_id = initial_packet_id;
  stream->on_packet_id = initial_packet_id;
  stream->transmit_callback = transmit_callback;
  spi_stream_load_tx(stream, initial_packet_id);
  return stream;
}

void spi_stream_load_tx(SPIStream* stream, uint32_t pkt_id) {
  spi_stream_construct_tx_packet(
      pkt_id,
      stream->spi_tx_buffer[pkt_id % SPI_BUFFER_TX_HISTORY],
      SPI_BUFFER_SIZE,
      stream->tx_buffer);
}

int delta_packet_id(uint32_t a, uint32_t b) {
  int difference = (int64_t)a - (int64_t)b;
  int modulo_difference = difference + ((int64_t)(1) << 32);
  return (abs(difference) < abs(modulo_difference)) ? difference : modulo_difference;
}

void spi_stream_transfer_data(SPIStream* stream, int error) {
  // verify the incoming data.
  int cksum_error = 0;
  uint32_t received_packet_id = spi_stream_verify_packet(
      stream->spi_rx_buffer, SPI_BUFFER_SIZE, &cksum_error);

//  printf("transfer start: (ck)%"PRIx32" (rec)%"PRIx32" (want)%"PRIx32" (on)%"PRIx32"\n", 
//      (uint32_t)cksum_error, received_packet_id, stream->waiting_for_packet_id,
//      stream->on_packet_id);

  // if the data is corrupt: resend the previous packet we are syncing on
  uint32_t pkt_to_send = stream->on_packet_id;

  // check if we got what we sent last go round
  if (!cksum_error && !error) {
    if (received_packet_id == stream->on_packet_id) {
      // read the data into the RX buffer
      int read_ok = 1;
      if (stream->on_packet_id == stream->waiting_for_packet_id) {
        read_ok = spi_stream_read_rx_packet(
            stream->spi_rx_buffer, stream->rx_buffer);
        if (read_ok) {
          stream->waiting_for_packet_id++;
          spi_stream_load_tx(stream, stream->waiting_for_packet_id);
        }
      }
      if (read_ok) {
        // advance to next packet, load the data.
        stream->on_packet_id++;
        pkt_to_send = stream->on_packet_id;
      }
    } else if (delta_packet_id(received_packet_id, stream->on_packet_id) < 0) {
      // The remote send an older packet than we were expecting - that means
      // it wants us to rewind to that packet
      pkt_to_send = received_packet_id;
      stream->on_packet_id = received_packet_id;
    } 
  }

//  printf("transfer end: (send)%"PRIx32" (want)%"PRIx32" (on)%"PRIx32"\n", 
//      pkt_to_send, stream->waiting_for_packet_id, stream->on_packet_id);

  stream->transmit_callback(
      (void*)(stream->spi_tx_buffer[pkt_to_send % SPI_BUFFER_TX_HISTORY]),
      (void*)stream->spi_rx_buffer,
      SPI_BUFFER_SIZE * sizeof(uint32_t));
}


void spi_stream_free(SPIStream* stream)
{
  free(stream);
}
