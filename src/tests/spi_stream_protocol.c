/*
 * =====================================================================================
 *
 *       Filename:  spi_stream_protocol.c
 *
 *    Description:  Tests of stream (un)escaping
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <string.h>

#include "minunit.h"
#include "adler32.h"
#include "spi_stream_protocol.h"


static char* test_escape_stream_data(void) {
  u32 input_stream[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    9
  };
  CircularBuffer* input_buffer = cbuffer_new();
  cbuffer_append(input_buffer, input_stream, 8);
  u32 output[16];
  escape_stream_data(output, 16, input_buffer);
 
  u32 expected_output[16] = {
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_OVERRUN,
    9,
    // end padding
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
  };
  mu_assert_eq("escaped content", 
      memcmp(output, expected_output, 16 * sizeof(u32)), 0);
  return 0;
}

static char* test_unescape(void) {
 
  u32 escaped_data[16] = {
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_OVERRUN,
    9,
    // end padding
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
  };

  CircularBuffer* dest = cbuffer_new();
  int error = unescape_data(dest, escaped_data, 16);

  u32 expected_data[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    9
  };

  mu_assert_eq("error", error, 0);
  mu_assert_eq("escaped size", cbuffer_size(dest), 8);
  mu_assert_eq("escaped content", 
      memcmp(dest->data, expected_data, 8 * sizeof(u32)), 0);
  return 0;
}

static char* test_idle_unescape(void) {

  u32 raw_data[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    5,
    6,
    7,
    SPI_STREAM_IDLE,
    9
  };

  u32 exp_data[4] = {
    5,
    6,
    7,
    9
  };

  CircularBuffer* output = cbuffer_new();

  int error = unescape_data(output, raw_data, 8);
  mu_assert_eq("idle", memcmp(output->data, exp_data, 4 * sizeof(u32)), 0);
  mu_assert_eq("error", error, 0);
  return 0;
}

static char* test_closure(char * msg, u32* data, u16 size, int experror) {
  CircularBuffer* input = cbuffer_new();
  cbuffer_append(input, data, size);

  u32 escaped_buffer[2 * size];
  escape_stream_data(escaped_buffer, 2 * size, input);

  if (experror & SPI_STREAM_ERR_REMOTE_UNDERRUN)
    escaped_buffer[2 * size - 1] = SPI_STREAM_UNDERRUN;
  if (experror & SPI_STREAM_ERR_REMOTE_OVERRUN)
    escaped_buffer[2 * size - 2] = SPI_STREAM_OVERRUN;

  CircularBuffer* output = cbuffer_new();
  int error = unescape_data(output, escaped_buffer, 2 * size);

  if (!experror) 
    mu_assert_eq(msg, memcmp(output->data, input->data, size * sizeof(u32)), 0);
  mu_assert_eq(msg, error, experror);

  cbuffer_free(input);
  cbuffer_free(output);
  return 0;
}

static char* test_all_normal(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("all_normal", data, 10, 0);
}

static char* test_all_escaped(void) {
  u32 data[4] = {
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN
  };
  return test_closure("all_escaped", data, 4, 0);
}

static char* test_underrun(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("underrun", data, 10, SPI_STREAM_ERR_REMOTE_UNDERRUN);
}

static char* test_overrun(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("overrun", data, 10, SPI_STREAM_ERR_REMOTE_OVERRUN);
}

static char* test_both_errors(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("both", data, 10, 
      SPI_STREAM_ERR_REMOTE_OVERRUN | SPI_STREAM_ERR_REMOTE_UNDERRUN);
}

static char* test_write_spi_stream_errors(void) {
  u32 data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int nerrors = write_spi_stream_errors(data, 
      SPI_STREAM_ERR_LOCAL_UNDERRUN);
  mu_assert_eq("1err", nerrors, 1);
  mu_assert_eq("1err content", data[0], SPI_STREAM_UNDERRUN);

  nerrors = write_spi_stream_errors(data, 
      SPI_STREAM_ERR_LOCAL_UNDERRUN |
      SPI_STREAM_ERR_LOCAL_RX_OVERFLOW
      );
  mu_assert_eq("2err", nerrors, 2);
  mu_assert_eq("2err content", data[0], SPI_STREAM_UNDERRUN);
  mu_assert_eq("2err content", data[1], SPI_STREAM_RX_OVERFLOW);

  nerrors = write_spi_stream_errors(data, 
      SPI_STREAM_ERR_LOCAL_UNDERRUN |
      SPI_STREAM_ERR_LOCAL_RX_OVERFLOW |
      SPI_STREAM_ERR_LOCAL_OVERRUN 
      );
  mu_assert_eq("3err", nerrors, 3);
  mu_assert_eq("3err content", data[0], SPI_STREAM_UNDERRUN);
  mu_assert_eq("3err content", data[1], SPI_STREAM_OVERRUN);
  mu_assert_eq("3err content", data[2], SPI_STREAM_RX_OVERFLOW);

  mu_assert_eq("remaining", data[3], 4);

  return 0;
}

static char* test_escape_stream(void) {
  u32 input_stream[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    9
  };
  CircularBuffer* input_buffer = cbuffer_new();
  cbuffer_append(input_buffer, input_stream, 8);
  u32 output[18];
  escape_stream(output, 18, input_buffer,
      SPI_STREAM_ERR_LOCAL_UNDERRUN |
      SPI_STREAM_ERR_LOCAL_RX_OVERFLOW |
      SPI_STREAM_ERR_LOCAL_OVERRUN 
      );
 
  u32 expected_output[18] = {
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    SPI_STREAM_RX_OVERFLOW,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_OVERRUN,
    9,
    // end padding
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    5 // checksum goes here
  };
  add_checksum(expected_output, 18);
  mu_assert_eq("escaped content", 
      memcmp(output, expected_output, 18 * sizeof(u32)), 0);
  return 0;
}

static char* test_unescape_overflow(void) {
 
  u32 escaped_data[16] = {
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_OVERRUN,
    9,
    // end padding
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
  };

  CircularBuffer* dest = cbuffer_new();
  int error = unescape_data(dest, escaped_data, 16);

  u32 expected_data[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    9
  };

  mu_assert_eq("error", error, 0);
  mu_assert_eq("escaped size", cbuffer_size(dest), 8);
  mu_assert_eq("escaped content", 
      memcmp(dest->data, expected_data, 8 * sizeof(u32)), 0);

  // lets see what happens if we don't have enough room in the cbuffer
  dest->pos = 0;
  dest->tail = IO_BUFFER_SIZE - 5;
  mu_assert_eq("freespace", cbuffer_freespace(dest), 4);
  error = unescape_data(dest, escaped_data, 16);
  mu_assert_eq("ovrflw error", error, SPI_STREAM_ERR_LOCAL_RX_OVERFLOW);
  // the output destination should be restored unmodified.
  mu_assert_eq("freespace after", cbuffer_freespace(dest), 4);
  
  return 0;
}

static char* test_corruption_detection(void) {
  u32 stream[15] = {
    SPI_STREAM_ESCAPE,
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_ESCAPE,
    SPI_STREAM_OVERRUN,
    9,
    // end padding
    SPI_STREAM_IDLE,
    SPI_STREAM_IDLE,
    5 // checksum goes here
  };
  add_checksum(stream, 15);
  CircularBuffer* dest = cbuffer_new();

  int error = unescape_stream(dest, stream, 15);

  u32 expected_data[8] = {
    SPI_STREAM_IDLE,
    SPI_STREAM_ESCAPE,
    5,
    6,
    7,
    SPI_STREAM_UNDERRUN,
    SPI_STREAM_OVERRUN,
    9
  };

  mu_assert_eq("error", error, 0);
  mu_assert_eq("escaped size", cbuffer_size(dest), 8);
  mu_assert_eq("escaped content", 
      memcmp(dest->data, expected_data, 8 * sizeof(u32)), 0);

  // now simulate data corruption
  stream[4] = 0;
  error = unescape_stream(dest, stream, 15);
  mu_assert_eq("error", error, SPI_STREAM_ERR_LOCAL_CKSUM);
  // the destination buffer is unchanged
  mu_assert_eq("escaped size", cbuffer_size(dest), 8);
  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== spi_stream_protocol tests ===\n");
  mu_run_test(test_escape_stream_data);
  mu_run_test(test_unescape);
  mu_run_test(test_idle_unescape);
  mu_run_test(test_all_normal);
  mu_run_test(test_all_escaped);
  mu_run_test(test_underrun);
  mu_run_test(test_overrun);
  mu_run_test(test_both_errors);
  mu_run_test(test_write_spi_stream_errors);
  mu_run_test(test_unescape_overflow);
  mu_run_test(test_escape_stream);
  mu_run_test(test_corruption_detection);
  return 0;
}
