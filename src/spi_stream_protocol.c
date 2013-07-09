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

#include "spi_stream_protocol.h"

int escape_stream_into(CircularBuffer* dest, const Buffer* src) {
  if (src->size > cbuffer_freespace(dest))
    return -1;
  int words_written = 0;
  for (int i = 0; i < src->size; ++i) {
    u32 data = src->data[i];
    if (data == SPI_STREAM_IDLE ||
        data == SPI_STREAM_UNDERRUN ||
        data == SPI_STREAM_OVERRUN ||
        data == SPI_STREAM_ESCAPE) {
      // this is the same as a control word, need to escape the data.
      cbuffer_push_back(dest, SPI_STREAM_ESCAPE);
      words_written++;
    }
    cbuffer_push_back(dest, src->data[i]);
    words_written++;
  }
  return words_written;
}


Buffer* unescape_stream(CircularBuffer* src, int* error) {
  // The buffer may actually be much smaller than this, but this is its
  // maximum size.
  int input_size = cbuffer_size(src);

  Buffer* output = buffer_new(0, input_size);
  *error = 0;

  int words_written = 0;
  int escape_active = 0;
  for (int i = 0; i < input_size; ++i) {
    u32 data = src->data[i];
    if (escape_active) {
      output->data[words_written] = data;
      escape_active = 0;
      words_written++;
    } else {
      switch (data) {
        case SPI_STREAM_IDLE:
          // nothing to report
          break;
        case SPI_STREAM_UNDERRUN:
          *error |= SPI_STREAM_ERR_UNDERRUN;
          break;
        case SPI_STREAM_OVERRUN:
          *error |= SPI_STREAM_ERR_OVERRUN;
          break;
        case SPI_STREAM_ESCAPE:
          escape_active = 1;
          break;
        default:
          output->data[words_written] = data;
          words_written++;
      }
    }
  }
  // Delete consumed data from src
  cbuffer_deletefront(src, input_size);
  // Free any unnecessary memory in the tail of output.
  buffer_resize(output, words_written);
  return output;
}
