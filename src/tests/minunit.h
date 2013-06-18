/*
 * Minimal unit testing macros.
 *
 * From http://www.jera.com/techinfo/jtns/jtn002.html
 *
 */

#ifndef MINUNIT_DZHNARJ8
#define MINUNIT_DZHNARJ8

#include <stdio.h>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_assert_eq(message, a, b) do { if ((a) != (b)) { printf("%s:%d - "#a" (%02x) != "#b" (%02x)\n", __FILE__, __LINE__, (unsigned int)(a), (unsigned int)(b)); return message; } } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
  if (message) { printf("Failure in "#test": "); return message; } } while (0)

#define HEX_PRINT(x) printf(#x" = %02x\n", (x))

extern int tests_run;
char * all_tests(void);

#endif /* end of include guard: MINUNIT_DZHNARJ8 */
