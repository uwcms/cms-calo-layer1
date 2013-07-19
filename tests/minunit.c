/*
 * =====================================================================================
 *
 *       Filename:  minunit.c
 *
 *    Description:  Entry point into a unit test.
 *
 *         Author:  Evan Friis (), evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#include "minunit.h"

int main(int argc, char **argv) {
  tests_run = 0;
  char *result = all_tests();
  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
