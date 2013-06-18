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

int tests_run;

char * all_tests(void) {
  printf("\n\n=== buffer tests ===\n");
  mu_run_test(test_cbuffer_new);
  mu_run_test(test_cbuffer_new);
  return 0;
}
