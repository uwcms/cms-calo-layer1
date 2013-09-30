#include "protocol.h"
#include "adler32.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

uint32_t spi_stream_verify_packet(const uint32_t* pkt, uint16_t pkt_size, int* cksum_error) {
  uint32_t pkt_id = pkt[0];
  *cksum_error = !verify_checksum(pkt, pkt_size);
  return pkt_id;
}

void spi_stream_construct_tx_packet(uint32_t pkt_id, uint32_t* pkt, uint16_t pkt_size, 
    CircularBuffer* src) {
  pkt[0] = pkt_id;
  uint32_t data_size = min(cbuffer_size(src), pkt_size - 3);
  // write all available data in.
  data_size = cbuffer_read(src, pkt + 2, data_size);
  cbuffer_deletefront(src, data_size);
  // write how much we actually put in.
  pkt[1] = data_size;
  // pad the end of the data, up to the checksum point.
  for (unsigned int i = data_size + 2; i < pkt_size - 1; ++i) {
    pkt[i] = 0xDEADBEEF;
  }
  // set the checksum
  add_checksum(pkt, pkt_size);
}

int spi_stream_read_rx_packet(const uint32_t* pkt, CircularBuffer* dest) {
  uint32_t data_size = pkt[1];
  if (cbuffer_freespace(dest) < data_size)
    return 0;
  cbuffer_append(dest, (void*)(pkt + 2), data_size);
  return 1;
}
