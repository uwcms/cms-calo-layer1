/*
 * Minimal unit testing macros.
 *
 * From http://www.jera.com/techinfo/jtns/jtn002.html
 *
 */

#include <stdio.h>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_assert_eq(message, a, b) do { if ((a) != (b)) { printf("%s:%d - "#a" (%02x) != "#b" (%02x)\n", __FILE__, __LINE__, (unsigned int)(a), (unsigned int)(b)); return message; } } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
  if (message) { printf("Failure in "#test": "); return message; } } while (0)

#define HEX_PRINT(x) printf(#x" = %02x\n", (x))

int tests_run;

static char * all_tests(void);

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
