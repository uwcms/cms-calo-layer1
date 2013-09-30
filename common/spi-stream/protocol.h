/*
 * =====================================================================================
 *
 *       Filename:  protocol.h
 *
 *    Description:  SPI Stream Protocol
 *
 *
 *    Each packet exchanged is a fixed length, corresponding to the SPI FIFO
 *    device sizes.  The first word is the "packet ID", which is incremented on
 *    each transition, and with the checksum serves as the error detection
 *    device.  The second word is the number of datawords, and must be smaller
 *    than the size of the packet minus 3.  The data payload then follows.
 *    Remaining space is padded by 0xDEADBEEF, with an Adler-32 checksum at the
 *    end.
 *
 *         word 0:              packet ID (1..2^32-1)
 *         word 1:              data size word
 *         word 2-n+2:          DATA
 *         ...                  DATA
 *         word N - n+2:        0xDEADBEEF
 *         ...                  0xDEADBEEF
 *         word N-1             Adler-32 checksum
 *
 *         Author:  Evan Friis, evan.friis@cern.ch Company:  UW Madison
 *
 * =====================================================================================
 */


#ifndef PROTOCOL_KS06GHI2
#define PROTOCOL_KS06GHI2

#include <stdint.h>
#include "circular_buffer.h"

// Verify packet checksum, and return the packet ID.
uint32_t spi_stream_verify_packet(const uint32_t* pkt, uint16_t pkt_size, int* cksum_error);

// Build an outgoing packet from <src>.
void spi_stream_construct_tx_packet(uint32_t pkt_id, uint32_t* pkt, uint16_t pkt_size,
    CircularBuffer* src);

// Read an incoming packet into <dest>.  Returns 1 on success, 0 on overflow of
// dest.
int spi_stream_read_rx_packet(const uint32_t* pkt, CircularBuffer* dest);

#endif /* end of include guard: PROTOCOL_KS06GHI2 */
