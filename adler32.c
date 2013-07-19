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
 
u32 adler32(const unsigned char *data, int len) {
  u32 a = 1, b = 0;

  /* Process each byte of the data in order */
  for (int index = 0; index < len; ++index) {
    a = (a + data[index]) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
  }

  return (b << 16) | a;
}

void add_checksum(u32* data, int len) {
  data[len - 1] = adler32((void*)data, sizeof(u32) * (len - 1));
}

int verify_checksum(const u32* data, int len) {
  return data[len - 1] == adler32((void*)data, sizeof(u32) * (len - 1));
}
