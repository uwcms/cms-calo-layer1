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

#define IO_BUFFER_SIZE (2048*1024)

#include "xil_types.h"

// todo: locking!

typedef struct {
  u32* data;
  u32 tail;
  u32 pos; // position w.r.t. <data> pointer where data starts.
} CircularBuffer;

// A non-circular buffer of words
typedef struct {
  u32* data;
  u32 size;
} Buffer;

// Build a new circular buffer, initialized to zero
CircularBuffer* cbuffer_new(void);

// Make a copy of a circular buffer
CircularBuffer* cbuffer_copy(CircularBuffer* from);

// Words in the buffer
u32 cbuffer_size(const CircularBuffer* buffer);

// Free memory associated to a circular buffer
void cbuffer_free(CircularBuffer*);

// Build a new buffer, from data. If data is 0, it will be initialized
// to zero.
Buffer* buffer_new(void* data, u32 size);

// Free memory associated to a buffer
void buffer_free(Buffer*);

// Resize a buffer.
void buffer_resize(Buffer*, u32 size);

// Get the word at a given index
u32 cbuffer_value_at(const CircularBuffer* buf, u32 idx);

// Check remaining space in the buffer
u32 cbuffer_freespace(const CircularBuffer*);

// Append <nwords> from <data> to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_append(CircularBuffer* buffer, void* data, u32 nwords);

// Append 1 word to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_push_back(CircularBuffer* buffer, u32 data);

// Read (up to) nwords from the buffer.  Returns words read.
u32 cbuffer_read(const CircularBuffer* buffer, u32* dest, u32 nwords);

// Read (up to) nwords from the buffer.  Returns number of bytes deleted.
u32 cbuffer_deletefront(CircularBuffer* buffer, u32 nwords);

// Pop (up to) nwords from the buffer.  Equivalent to read + deletefront.
Buffer* cbuffer_pop(CircularBuffer* buffer, u32 nwords);

#endif
