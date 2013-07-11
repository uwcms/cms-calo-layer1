/*
 * =============================================================================
 *
 *       Filename:  spi_stream_protocol.h
 *
 *    Description:  Control flow characters to wrap a byte stream over SPI.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =============================================================================
 */


#ifndef SPI_STREAM_PROTOCOL_W8O7975
#define SPI_STREAM_PROTOCOL_W8O7975

#include "circular_buffer.h"

// Control words
#define SPI_STREAM_IDLE         0xBBBBBBBB
#define SPI_STREAM_ESCAPE       0xBEEFCAFE
// Control words used to indicate a local problem to the remote side.
// Device driver SPI buffer over/underrun errors
#define SPI_STREAM_UNDERRUN     0xBEEFFACE
#define SPI_STREAM_OVERRUN      0xDEADBEEF
// Local RX overflow
#define SPI_STREAM_RX_OVERFLOW  0xDEADFACE
// Receive buffer checksum error
#define SPI_STREAM_RX_CKSUM     0xFACEBEEF

// 32 bit error flags.  Local errors are bottom two bytes, remote in top two.
#define SPI_STREAM_ERR_LOCAL_UNDERRUN (1 << 0)
#define SPI_STREAM_ERR_LOCAL_OVERRUN (1 << 1)
#define SPI_STREAM_ERR_LOCAL_RX_OVERFLOW (1 << 2)
#define SPI_STREAM_ERR_LOCAL_CKSUM (1 << 3)
#define SPI_STREAM_ERR_LOCAL_MASK 0x0000FFFF

#define SPI_STREAM_ERR_REMOTE_UNDERRUN (1 << 16)
#define SPI_STREAM_ERR_REMOTE_OVERRUN (1 << 17)
#define SPI_STREAM_ERR_REMOTE_RX_OVERFLOW (1 << 18)
#define SPI_STREAM_ERR_REMOTE_CKSUM (1 << 19)
#define SPI_STREAM_ERR_REMOTE_MASK 0xFFFF0000

// Transform a stream of words to TX format, escaping any control characters,
// into a fixed size word buffer at <dest> of <dest_size>.  If the <src> buffer
// cannot fill the destination, SPI_STREAM_IDLE characters are used to pad.
// This can then be transmitted over the SPI interface.  Returns number of words
// successfully consumed from src, which is modified.  This function should nod
// be directly used, use escape_stream below.
int escape_stream_data(u32* dest, u16 dest_size, CircularBuffer* src);

// Same as above, but local errors are first inserted into the data stream.
// <local_errors> are the OR-ed local error flags from the previous exchange.
// The final word of <dest> is the Adler-32 checksum.
int escape_stream(u32* dest, u16 dest_size, CircularBuffer* src, int local_errors);

// Demux error codes in <local_errors> to error flag words in to <dest>.
// Returns number of words inserted.
int write_spi_stream_errors(u32* dest, int local_errors);

// Transform a stream of RXed words, removing any control characters, and
// unescaping any control characters in the real data.  The returned error will
// be set to zero if no errors are detected, or SPI_STREAM_ERR_UNDERRUN,
// SPI_STREAM_ERR_OVERRUN.  If the local <dest> buffer
// cannot handle all of the data, a SPI_STREAM_ERR_LOCAL_RX_OVERFLOW will be
// immediately returned.  In this case, <dest> is rolled back to its previous
// state.
int unescape_data(CircularBuffer* dest, u32* src, u16 src_size);

// As above, but first checks the checksum in the last word of src. (then passes
// all but the last word to the unescape_data function).
int unescape_stream(CircularBuffer* dest, u32* src, u16 src_size);

#endif /* end of include guard: SPI_STREAM_PROTOCOL_W8O7975 */
