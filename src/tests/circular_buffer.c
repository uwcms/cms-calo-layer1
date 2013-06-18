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

static char* test_cbuffer_append_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, "BuckyBadger", 11);
  mu_assert_eq("pos", mybuf->pos, IO_BUFFER_SIZE - 5);
  mu_assert_eq("size", mybuf->size, 11);
  mu_assert_eq("tail content", memcmp(mybuf->data + mybuf->pos, "Bucky", 5), 0);
  // make sure we aren't going to var.
  mu_assert_eq("tail content sanity", memcmp(mybuf->data + IO_BUFFER_SIZE-1, "y", 1), 0);
  mu_assert_eq("head content", memcmp(mybuf->data, "Badger", 6), 0);
  cbuffer_append(mybuf, "Bucky", 5);
  mu_assert_eq("size", mybuf->size, 16);
  mu_assert_eq("content", memcmp(mybuf->data, "BadgerBucky", 11), 0);
  return 0;
}

static char* test_cbuffer_read(void) {
  CircularBuffer* mybuf = cbuffer_new();
  cbuffer_append(mybuf, "BuckyBadger", 11);
  Buffer* readout = cbuffer_read(mybuf, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, "BuckyBadger", 11), 0);
  // if we ask for too much it cbuffer gives us what it has.
  Buffer* readout2 = cbuffer_read(mybuf, 30);
  mu_assert_eq("size2", readout2->size, 11);
  mu_assert_eq("content2", memcmp(readout2->data, "BuckyBadger", 11), 0);
  return 0;
}

static char* test_cbuffer_read_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, "BuckyBadger", 11);
  Buffer* readout = cbuffer_read(mybuf, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, "BuckyBadger", 11), 0);
  return 0;
}

static char* test_cbuffer_delete_front(void) {
  CircularBuffer* mybuf = cbuffer_new();
  cbuffer_append(mybuf, "BuckyBadger", 11);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 5);
  mu_assert_eq("size", mybuf->size, 6);

  Buffer* readout = cbuffer_read(mybuf, 6);
  mu_assert_eq("remaining content", memcmp(readout->data, "Badger", 6), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 11);
  mu_assert_eq("size2", mybuf->size, 0);
  return 0;
}

static char* test_cbuffer_delete_front_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, "BuckyBadger", 11);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", mybuf->size, 6);

  Buffer* readout = cbuffer_read(mybuf, 6);
  mu_assert_eq("remaining content", memcmp(readout->data, "Badger", 6), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 6);
  mu_assert_eq("size2", mybuf->size, 0);
  return 0;
}

static char* test_cbuffer_pop(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, "BuckyBadger", 11);

  Buffer* bucky = cbuffer_pop(mybuf, 5);
  mu_assert_eq("size", mybuf->size, 6);
  mu_assert_eq("content", memcmp(bucky->data, "Bucky", 5), 0);

  Buffer* badger = cbuffer_pop(mybuf, 6);
  mu_assert_eq("size2", mybuf->size, 0);
  mu_assert_eq("content2", memcmp(badger->data, "Badger", 6), 0);

  // if we pop an empty collection, we get nothing.
  Buffer* empty = cbuffer_pop(mybuf, 10);
  mu_assert_eq("size3", empty->size, 0);

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
  mu_run_test(test_cbuffer_append_wraps);
  mu_run_test(test_cbuffer_read);
  mu_run_test(test_cbuffer_read_wraps);
  mu_run_test(test_cbuffer_delete_front);
  mu_run_test(test_cbuffer_delete_front_wraps);
  mu_run_test(test_cbuffer_pop);
  return 0;
}
