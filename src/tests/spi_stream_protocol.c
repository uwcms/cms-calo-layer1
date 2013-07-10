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
#include "spi_stream_protocol.h"


static char* test_escape_stream_into(void) {
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
  escape_stream_into(output, 16, input_buffer);
 
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
  int error = unescape_stream_into(dest, escaped_data, 16);

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

  int error = unescape_stream_into(output, raw_data, 8);
  mu_assert_eq("idle", memcmp(output->data, exp_data, 4 * sizeof(u32)), 0);
  mu_assert_eq("error", error, 0);
  return 0;
}

static char* test_closure(char * msg, u32* data, u16 size, int experror) {
  CircularBuffer* input = cbuffer_new();
  cbuffer_append(input, data, size);

  u32 escaped_buffer[2 * size];
  escape_stream_into(escaped_buffer, 2 * size, input);

  if (experror & SPI_STREAM_ERR_REMOTE_UNDERRUN)
    escaped_buffer[2 * size - 1] = SPI_STREAM_UNDERRUN;
  if (experror & SPI_STREAM_ERR_REMOTE_OVERRUN)
    escaped_buffer[2 * size - 2] = SPI_STREAM_OVERRUN;

  CircularBuffer* output = cbuffer_new();
  int error = unescape_stream_into(output, escaped_buffer, 2 * size);

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



int tests_run;

char * all_tests(void) {
  printf("\n\n=== spi_stream_protocol tests ===\n");
  mu_run_test(test_escape_stream_into);
  mu_run_test(test_unescape);
  mu_run_test(test_idle_unescape);
  mu_run_test(test_all_normal);
  mu_run_test(test_all_escaped);
  mu_run_test(test_underrun);
  mu_run_test(test_overrun);
  mu_run_test(test_both_errors);
  return 0;
}
