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
#include <inttypes.h>

#define MOD_ADLER 65521
 
uint32_t adler32(const unsigned char *data, int len) {
  uint32_t a = 1, b = 0;

  /* Process each byte of the data in order */
  for (int index = 0; index < len; ++index) {
    a = (a + data[index]) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
  }

  return (b << 16) | a;
}

void add_checksum(uint32_t* data, int len) {
  data[len - 1] = adler32((void*)data, sizeof(uint32_t) * (len - 1));
}

int verify_checksum(const uint32_t* data, int len) {
  return data[len - 1] == adler32((void*)data, sizeof(uint32_t) * (len - 1));
}
