/*
 * =====================================================================================
 *
 *       Filename:  test_circular_buffer.c
 *
 *    Description:  Tests of buffer functionality.
 *
 *         Author:  Evan Friis (), evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <string.h>

#include "minunit.h"
#include "circular_buffer.h"

static char* test_cbuffer_new(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mu_assert_eq("size", mybuf->size, 0);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("freespace", cbuffer_freespace(mybuf), IO_BUFFER_SIZE);
  mu_assert_eq("init is zero", (mybuf->data[0]), 0);
  return 0;
}

static char* test_cbuffer_free(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // doesn't crash
  cbuffer_free(mybuf);
  return 0;
}

static char* test_buffer_new(void) {
  Buffer* mybuf = buffer_new("BuckyBadger", 5);
  mu_assert_eq("size", mybuf->size, 5);
  mu_assert_eq("content", memcmp(mybuf->data, "Bucky", 5), 0);
  return 0;
}

static char* test_buffer_free(void) {
  Buffer* mybuf = buffer_new(NULL, 20);
  // doesn't crash
  buffer_free(mybuf);
  return 0;
}

static char* test_cbuffer_append(void) {
  CircularBuffer* mybuf = cbuffer_new();
  cbuffer_append(mybuf, "BuckyBadger", 11);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", mybuf->size, 11);
  mu_assert_eq("content", memcmp(mybuf->data, "BuckyBadger", 11), 0);
  cbuffer_append(mybuf, "Bucky", 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", mybuf->size, 16);
  mu_assert_eq("content", memcmp(mybuf->data, "BuckyBadgerBucky", 16), 0);
  return 0;
}


int tests_run;

char * all_tests(void) {
  printf("\n\n=== buffer tests ===\n");
  mu_run_test(test_cbuffer_new);
  mu_run_test(test_cbuffer_free);
  mu_run_test(test_buffer_new);
  mu_run_test(test_buffer_free);
  mu_run_test(test_cbuffer_append);
  return 0;
}
