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
  Buffer* input_buffer = buffer_new(input_stream, 8);
  CircularBuffer* output = cbuffer_new();
  escape_stream_into(output, input_buffer);
 
  u32 expected_output[12] = {
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
    9
  };
  mu_assert_eq("escaped size", cbuffer_size(output), 12);
  mu_assert_eq("escaped content", 
      memcmp(output->data, expected_output, 12 * sizeof(u32)), 0);
  return 0;
}

static char* test_unescape(void) {
  CircularBuffer* src = cbuffer_new();
 
  u32 escaped_data[12] = {
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
    9
  };
  cbuffer_append(src, escaped_data, 12);

  int error = -1;

  Buffer* output = unescape_stream(src, &error);

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
  Buffer* expected_buffer = buffer_new(expected_data, 8);

  mu_assert_eq("error", error, 0);
  mu_assert_eq("consumed size", cbuffer_size(src), 0);
  mu_assert_eq("escaped size", output->size, 8);
  mu_assert_eq("escaped content", 
      memcmp(expected_buffer->data, output->data, 8 * sizeof(u32)), 0);
  return 0;
}

static char* test_closure(char * msg, u32* data, u16 size, int experror) {
  Buffer* src = buffer_new(data, size);
  CircularBuffer* escaped = cbuffer_new();
  escape_stream_into(escaped, src);
  if (experror == SPI_STREAM_ERR_UNDERRUN)
    cbuffer_push_back(escaped, SPI_STREAM_UNDERRUN);
  if (experror == SPI_STREAM_ERR_OVERRUN)
    cbuffer_push_back(escaped, SPI_STREAM_OVERRUN);
  int error = -1;
  Buffer* unescaped = unescape_stream(escaped, &error);
  if (!experror) 
    mu_assert_eq(msg, memcmp(unescaped->data, src->data, size * sizeof(u32)), 0);
  mu_assert_eq(msg, error, experror);
  buffer_free(src);
  cbuffer_free(escaped);
  buffer_free(unescaped);
  return 0;
}

static char* test_idle_unescape(void) {
  CircularBuffer* input_raw_stream = cbuffer_new();

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

  cbuffer_append(input_raw_stream, raw_data, 8);

  u32 exp_data[4] = {
    5,
    6,
    7,
    9
  };

  int error = -1;
  Buffer* unescaped = unescape_stream(input_raw_stream, &error);
  mu_assert_eq("idle", memcmp(unescaped->data, exp_data, 4 * sizeof(u32)), 0);
  mu_assert_eq("error", error, 0);
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
  return test_closure("underrun", data, 10, SPI_STREAM_ERR_UNDERRUN);
}

static char* test_overrun(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("overrun", data, 10, SPI_STREAM_ERR_OVERRUN);
}

static char* test_both_errors(void) {
  u32 data[10] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };
  return test_closure("both", data, 10, 
      SPI_STREAM_ERR_OVERRUN | SPI_STREAM_ERR_OVERRUN);
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
