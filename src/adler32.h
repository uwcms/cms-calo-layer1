/*
 * =============================================================================
 *
 *       Filename:  adler32.h
 *
 *    Description:  Adler-32 Checksum.  Adapted from
 *                  https://en.wikipedia.org/wiki/Adler-32
 *
 * =============================================================================
 */

#ifndef ADLER32_9IRJ0M8H
#define ADLER32_9IRJ0M8H

#include <inttypes.h>

// Compute the Adler-32 checksum.
/* where data is the location of the data in physical memory and
 * len is the length of the data in bytes */
uint32_t adler32(unsigned char *data, int32_t len);

#endif /* end of include guard: ADLER32_9IRJ0M8H */
