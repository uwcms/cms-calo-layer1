/*
 * =====================================================================================
 *
 *       Filename:  spi_stream_protocol.h
 *
 *    Description:  Control flow characters to wrap a byte stream over SPI.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */


#ifndef SPI_STREAM_PROTOCOL_W8O7975
#define SPI_STREAM_PROTOCOL_W8O7975

#include "circular_buffer.h"

// control words
#define SPI_STREAM_IDLE         0xBBBBBBBB
#define SPI_STREAM_ESCAPE       0xBEEFCAFE
#define SPI_STREAM_UNDERRUN     0xBEEFFACE
#define SPI_STREAM_OVERRUN      0xDEADBEEF

// error flags
#define SPI_STREAM_ERR_UNDERRUN (1 << 0)
#define SPI_STREAM_ERR_OVERRUN (1 << 1)
#define SPI_STREAM_ERR_LOCAL_RX_OVERFLOW (1 << 2)

// Transform a stream of words to TX, escaping any control characters, into a
// fixed size word buffer at <dest> of <dest_size>.  If the <src> buffer cannot
// fill the destination, SPI_STREAM_IDLE characters are used to pad.
// This can then be transmitted over the SPI interface.  Returns number
// of words successfully consumed from src, which is modified.
int escape_stream_into(u32* dest, u16 dest_size, CircularBuffer* src);

// Transform a stream of RXed words, removing any control characters,
// and unescaping any control characters in the real data.
// The returned error will be set to zero if no errors are detected,
// or SPI_STREAM_ERR_UNDERRUN or SPI_STREAM_ERR_OVERRUN.  If the local <dest>
// buffer cannot handle all of the data, a SPI_STREAM_ERR_LOCAL_RX_OVERFLOW
// will be immediately returned.
int unescape_stream_into(CircularBuffer* dest, u32* src, u16 src_size);

#endif /* end of include guard: SPI_STREAM_PROTOCOL_W8O7975 */
