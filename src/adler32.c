/*
 * =====================================================================================
 *
 *       Filename:  adler32.c
 *
 *         Author:  Adapted from public domain code at 
 *                  https://en.wikipedia.org/wiki/Adler-32
 *
 * =====================================================================================
 */

#include "adler32.h"

#define MOD_ADLER 65521
 
uint32_t adler32(unsigned char *data, int32_t len) {
  uint32_t a = 1, b = 0;
  int32_t index;

  /* Process each byte of the data in order */
  for (index = 0; index < len; ++index) {
    a = (a + data[index]) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
  }

  return (b << 16) | a;
}
