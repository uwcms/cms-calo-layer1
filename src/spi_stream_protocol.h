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
#define SPI_STREAM_IDLE         0xBBBB
#define SPI_STREAM_ESCAPE       0xBEEF
#define SPI_STREAM_UNDERRUN     0xDEAD
#define SPI_STREAM_OVERRUN      0xFACE

// error flags
#define SPI_STREAM_ERR_UNDERRUN (1 << 0)
#define SPI_STREAM_ERR_OVERRRUN (1 << 1)

// Transform a stream of words to TX, escaping any control characters.
// This can then be transmitted over the SPI interface.  Returns number
// of bytes successfully consumed.
int escape_stream_into(CircularBuffer* dest, const Buffer* src);

// Transform a stream of RXed words, removing any control characters,
// and unescaping any control characters in the real data.
// error will be set to zero if no errors are detected,
// or SPI_STREAM_ERR_UNDERRUN or SPI_STREAM_ERR_OVERRUN.
// The bytes are consumed from src (it is modified).
Buffer* unescape_stream(CircularBuffer* src, int* error);

#endif /* end of include guard: SPI_STREAM_PROTOCOL_W8O7975 */
