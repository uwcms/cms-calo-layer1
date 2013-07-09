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
  mu_assert_eq("size", cbuffer_size(mybuf), 0);
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
  u32 test_data[5] = {0, 1, 2, 3, 4};
  Buffer* mybuf = buffer_new(test_data, 4);
  mu_assert_eq("size", mybuf->size, 4);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 4 * sizeof(u32)), 0);
  return 0;
}

static char* test_buffer_free(void) {
  Buffer* mybuf = buffer_new(NULL, 20);
  // doesn't crash
  buffer_free(mybuf);
  return 0;
}

static char* test_buffer_resize(void) {
  u32 test_data[5] = {0, 1, 2, 3, 4};
  Buffer* mybuf = buffer_new(test_data, 5);
  buffer_resize(mybuf, 3);
  mu_assert_eq("size", mybuf->size, 3);
  mu_assert_eq("content", 
      memcmp(mybuf->data, test_data, 3 * sizeof(u32)), 0);
  return 0;
}

static char* test_cbuffer_append(void) {
  CircularBuffer* mybuf = cbuffer_new();
  u32 test_data[5] = {0, 1, 2, 3, 4};
  cbuffer_append(mybuf, test_data, 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 5);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 5 * sizeof(u32)), 0);

  u32 test_data2[3] = {6, 7, 8};
  cbuffer_append(mybuf, test_data2, 3);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 8);
  mu_assert_eq("content2", memcmp(mybuf->data, test_data, 5 * sizeof(u32)), 0);

  mu_assert_eq("content3", memcmp(&(mybuf->data[5]), 
        test_data2, 3 * sizeof(u32)), 0);
  return 0;
}

static char* test_cbuffer_append_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);

  mu_assert_eq("pos", mybuf->pos, IO_BUFFER_SIZE - 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 11);
  mu_assert_eq("tail content", memcmp(
        &(mybuf->data[mybuf->pos]),
        test_data, 
        5 * sizeof(u32)), 0);

  // make sure we aren't trashing the memory after the buffer.
  //mu_assert_eq("tail content sanity", mybuf->data[IO_BUFFER_SIZE-1], 4);

  mu_assert_eq("head content", memcmp(
        mybuf->data, 
        test_data + 5, 
        6 * sizeof(u32)), 0);

  u32 test_data2[3] = {11, 12, 13};
  cbuffer_append(mybuf, test_data2, 3);
  mu_assert_eq("size", cbuffer_size(mybuf), 14);
  mu_assert_eq("content", memcmp(&(mybuf->data[6]), test_data2, 3), 0);
  return 0;
}

static char* test_cbuffer_read(void) {
  CircularBuffer* mybuf = cbuffer_new();
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  Buffer* readout = cbuffer_read(mybuf, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, test_data, 11 * sizeof(u32)), 0);
  // if we ask for too much it cbuffer gives us what it has.
  Buffer* readout2 = cbuffer_read(mybuf, 30);
  mu_assert_eq("size2", readout2->size, 11);
  mu_assert_eq("content2", memcmp(readout2->data, test_data, 11 * sizeof(u32)), 0);
  return 0;
}

static char* test_cbuffer_read_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  Buffer* readout = cbuffer_read(mybuf, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, test_data, 11 * sizeof(u32)), 0);
  return 0;
}

static char* test_cbuffer_delete_front(void) {
  CircularBuffer* mybuf = cbuffer_new();
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 11 * sizeof(u32)), 0);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 5);
  mu_assert_eq("item1", mybuf->data[mybuf->pos+1], 6);
  mu_assert_eq("item2", mybuf->data[mybuf->pos+2], 7);

  mu_assert_eq("remaining content in cbuffer", memcmp(
        &(mybuf->data[5]), 
        test_data + 5, 6 * sizeof(u32)), 0);

  Buffer* readout = cbuffer_read(mybuf, 6);
  mu_assert_eq("remaining content", memcmp(
        readout->data, 
        (test_data + 5), 
        6 * sizeof(u32)), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 11);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);
  return 0;
}

static char* test_cbuffer_delete_front_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, test_data, 11);
  mu_assert_eq("content", memcmp(&(mybuf->data[mybuf->pos]), 
        test_data, 5 * sizeof(u32)), 0);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 5);
  mu_assert_eq("item1", mybuf->data[mybuf->pos+1], 6);
  mu_assert_eq("item2", mybuf->data[mybuf->pos+2], 7);

  mu_assert_eq("remaining content in cbuffer", memcmp(
        &(mybuf->data[0]), 
        test_data + 5, 6 * sizeof(u32)), 0);

  Buffer* readout = cbuffer_read(mybuf, 6);
  mu_assert_eq("remaining content", memcmp(
        readout->data, 
        (test_data + 5), 
        6 * sizeof(u32)), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 6);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);
  return 0;
}


static char* test_cbuffer_pop(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  u32 test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);

  Buffer* bucky = cbuffer_pop(mybuf, 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);
  mu_assert_eq("content", memcmp(bucky->data, test_data, 
        5*sizeof(u32)), 0);

  Buffer* badger = cbuffer_pop(mybuf, 6);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);
  mu_assert_eq("content2", 
      memcmp(badger->data, test_data + 5, 6), 0);

  // if we pop an empty collection, we get nothing.
  Buffer* empty = cbuffer_pop(mybuf, 10);
  mu_assert_eq("size3", empty->size, 0);

  return 0;
}

static char* test_cbuffer_push_back(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 2;
  mybuf->tail = IO_BUFFER_SIZE - 2;
  cbuffer_push_back(mybuf, 0xDEADBEEF);
  cbuffer_push_back(mybuf, 0xBEEFFACE);
  cbuffer_push_back(mybuf, 0xDEADFACE);

  mu_assert_eq("size", cbuffer_size(mybuf), 3);
  mu_assert_eq("pos", mybuf->pos, IO_BUFFER_SIZE-2);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 0xDEADBEEF);
  mu_assert_eq("item1", mybuf->data[mybuf->pos + 1], 0xBEEFFACE);
  mu_assert_eq("item2", mybuf->data[0], 0xDEADFACE);

  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== buffer tests ===\n");
  mu_run_test(test_cbuffer_new);
  mu_run_test(test_cbuffer_free);
  mu_run_test(test_buffer_new);
  mu_run_test(test_buffer_free);
  mu_run_test(test_buffer_resize);
  mu_run_test(test_cbuffer_append);
  mu_run_test(test_cbuffer_append_wraps);
  mu_run_test(test_cbuffer_read);
  mu_run_test(test_cbuffer_read_wraps);
  mu_run_test(test_cbuffer_delete_front);
  mu_run_test(test_cbuffer_delete_front_wraps);
  mu_run_test(test_cbuffer_pop);
  mu_run_test(test_cbuffer_push_back);
  return 0;
}
