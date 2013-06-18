/*
 * =====================================================================================
 *
 *       Filename:  helloworld.c
 *
 *    Description:  Just making sure the unit test stuff works.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */


#include "minunit.h"

static char* test_helloworld_pass(void) {
  return 0;
}

static char* test_helloworld_fail(void) {
  return "hello world";
}

static char * all_tests(void) {
  printf("\n\n=== hello world ===\n");
  mu_run_test(test_helloworld_pass);
  mu_run_test(test_helloworld_fail);
  return 0;
}
