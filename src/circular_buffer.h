/*
 * ============================================================================
 *
 *       Filename:  circular_buffer.h
 *
 *    Description:  A circular buffer of words.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * ============================================================================
 */

#ifndef CIRCULAR_BUFFER_SKY7YN38
#define CIRCULAR_BUFFER_SKY7YN38

#define IO_BUFFER_SIZE 2048

#include "xil_types.h"

// todo: locking!

typedef struct {
  u32* data;
  u32 size;
  u32 pos; // position w.r.t. <data> pointer where data starts.
} CircularBuffer;

// A non-circular buffer of words
typedef struct {
  u32* data;
  u32 size;
} Buffer;

// Build a new circular buffer, initialized to zero
CircularBuffer* cbuffer_new(void);

// Free memory associated to a circular buffer
void cbuffer_free(CircularBuffer*);

// Build a new buffer, from data. If data is 0, it will be initialized
// to zero.
Buffer* buffer_new(void* data, u16 size);

// Free memory associated to a buffer
void buffer_free(Buffer*);

// Check remaining space in the buffer
int cbuffer_freespace(CircularBuffer*);

// Append <nwords> from <data> to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_append(CircularBuffer* buffer, void* data, u16 nwords);

// Append 1 word to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_push_back(CircularBuffer* buffer, u32 data);

// Read (up to) nwords from the buffer.
Buffer* cbuffer_read(CircularBuffer* buffer, u16 nwords);

// Read (up to) nwords from the buffer.  Returns number of bytes deleted.
int cbuffer_deletefront(CircularBuffer* buffer, u16 nwords);

// Pop (up to) nwords from the buffer.  Equivalent to read + deletefront.
Buffer* cbuffer_pop(CircularBuffer* buffer, u16 nwords);

#endif
