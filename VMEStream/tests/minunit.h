/* file: minunit.h */
 #define mu_assert(message, test) do { if (!(test)) return message; } while (0)
 #define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
 #define mu_assert_eq(message, a, b) do { if ((a) != (b)) { printf("%s:%d - "#a" (%02x) != "#b" (%02x)\n", __FILE__, __LINE__, (unsigned int)(a), (unsigned int)(b)); return message; } } while (0)
 extern int tests_run;
