/*
 * =====================================================================================
 *
 *       Filename:  spi_stream_protocol.c
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#include <string.h>
#include <stdio.h>

#include "spi_stream_protocol.h"

int escape_stream_into(u32* dest, u16 dest_size, CircularBuffer* src) {

  int words_written = 0;
  int words_read = 0;
  int input_size = cbuffer_size(src);
  // We only go up to dest_size - 1, in the case that we need to escape the 
  // last word, turning it into two words.
  for (int i = 0; i < input_size && words_written < dest_size - 1; ++i) {
    u32 data = cbuffer_value_at(src, i);
    words_read++;
    if (data == SPI_STREAM_IDLE ||
        data == SPI_STREAM_UNDERRUN ||
        data == SPI_STREAM_OVERRUN ||
        data == SPI_STREAM_RX_OVERFLOW || 
        data == SPI_STREAM_ESCAPE) {
      // the payload data is the same as a control word, we need to escape it.
      dest[words_written] = SPI_STREAM_ESCAPE;
      words_written++;
    }
    dest[words_written] = data;
    words_written++;
  }
  // Pad the end with IDLE characters.  There is always at least one.
  for (; words_written < dest_size; ++words_written) {
    dest[words_written] = SPI_STREAM_IDLE;
  }
  cbuffer_deletefront(src, words_read);
  return words_read;
}

int escape_stream_into_werrors(u32* dest, u16 dest_size, CircularBuffer* src, 
    int local_errors) {
  int nerrorflags = write_spi_stream_errors(dest, local_errors);
  dest += nerrorflags;
  dest_size -= nerrorflags;
  return escape_stream_into(dest, dest_size, src);
}

int write_spi_stream_errors(u32* dest, int local_errors) {
  int nerrors = 0;
  if (local_errors & SPI_STREAM_ERR_LOCAL_UNDERRUN) {
    dest[nerrors++] = SPI_STREAM_UNDERRUN;
  }
  if (local_errors & SPI_STREAM_ERR_LOCAL_OVERRUN) {
    dest[nerrors++] = SPI_STREAM_OVERRUN;
  }
  if (local_errors & SPI_STREAM_ERR_LOCAL_RX_OVERFLOW) {
    dest[nerrors++] = SPI_STREAM_RX_OVERFLOW;
  }
  return nerrors;
}


int unescape_stream_into(CircularBuffer* dest, u32* src, u16 src_size) {
  int error = 0;
  int words_written = 0;
  int escape_active = 0;
  // if we overflow the local RX buffer, we roll back to the previous unmodified 
  // state
  int original_tail = dest->tail;
  for (int i = 0; i < src_size; ++i) {
    u32 data = src[i];
    if (escape_active) {
      if (cbuffer_freespace(dest) < 1) {
        dest->tail = original_tail;
        return error |= SPI_STREAM_ERR_LOCAL_RX_OVERFLOW;
      }
      cbuffer_push_back(dest, data);
      escape_active = 0;
      words_written++;
    } else {
      switch (data) {
        case SPI_STREAM_IDLE:
          // nothing to report
          break;
        case SPI_STREAM_UNDERRUN:
          error |= SPI_STREAM_ERR_REMOTE_UNDERRUN;
          break;
        case SPI_STREAM_OVERRUN:
          error |= SPI_STREAM_ERR_REMOTE_OVERRUN;
          break;
        case SPI_STREAM_RX_OVERFLOW:
          error |= SPI_STREAM_ERR_REMOTE_RX_OVERFLOW;
          break;
        case SPI_STREAM_ESCAPE:
          escape_active = 1;
          break;
        default:
          if (cbuffer_freespace(dest) < 1) {
            dest->tail = original_tail;
            return error |= SPI_STREAM_ERR_LOCAL_RX_OVERFLOW;
          }
          cbuffer_push_back(dest, data);
          words_written++;
      }
    }
  }
  return error;
}
