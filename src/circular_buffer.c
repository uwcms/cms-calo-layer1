#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "circular_buffer.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

CircularBuffer* cbuffer_new(void) {
  CircularBuffer* output = malloc(sizeof(CircularBuffer));
  output->data = malloc(IO_BUFFER_SIZE * sizeof(u32));
  output->tail = 0;
  output->pos = 0;
  return output;
}

CircularBuffer* cbuffer_copy(CircularBuffer* from) {
  CircularBuffer* output = malloc(sizeof(CircularBuffer));
  output->data = malloc(IO_BUFFER_SIZE * sizeof(u32));
  memcpy(output->data, from->data, IO_BUFFER_SIZE * sizeof(u32));
  output->tail = from->tail;
  output->pos = from->pos;
  return output;
}

u32 cbuffer_size(const CircularBuffer* buffer) {
  if (buffer->pos <= buffer->tail) 
    return buffer->tail - buffer->pos;
  return buffer->tail + IO_BUFFER_SIZE - buffer->pos;
}

void cbuffer_free(CircularBuffer* tokill) {
  free(tokill->data);
  free(tokill);
}

Buffer* buffer_new(void* data, u32 size) {
  Buffer* output = malloc(sizeof(Buffer));
  output->data = malloc(size * sizeof(u32));
  output->size = size;
  if (data) {
    memcpy(output->data, data, size * sizeof(u32));
  } else {
    memset(output->data, 0, size * sizeof(u32));
  }
  return output;
}

void buffer_free(Buffer* buf) {
  free(buf->data);
  free(buf);
}

void buffer_resize(Buffer* buf, u32 size) {
  buf->data = realloc(buf->data, size * sizeof(u32));
  buf->size = size;
}

u32 cbuffer_value_at(const CircularBuffer* buf, u32 idx) {
  u32 actual_idx = (buf->pos + idx) % IO_BUFFER_SIZE;
  return buf->data[actual_idx];
}

u32 cbuffer_freespace(const CircularBuffer* buf) {
  return IO_BUFFER_SIZE - cbuffer_size(buf) - 1;
}

int cbuffer_append(CircularBuffer* buffer, void* data, u32 nwords) {
  u32 freespace = cbuffer_freespace(buffer);
  if (freespace < nwords) {
    return -1;
  }

  // bytes available on "tail" until we hit the absolute end of the memory.
  // we know from the freespace check that we can't overwrite the head with
  // only <nwords>.
  u32 tail_length = IO_BUFFER_SIZE - buffer->tail;

  memcpy(
      &(buffer->data[buffer->tail]),
      data, 
      sizeof(u32) * min(nwords, tail_length));

  // if we didn't write everything, we need to wrap around to the beginning.
  if (tail_length < nwords) {
    memcpy(
        buffer->data, 
        data + sizeof(u32) * tail_length,
        sizeof(u32) * (nwords - tail_length));
  }
  buffer->tail = (buffer->tail + nwords) % IO_BUFFER_SIZE;
  return 0;
}

int cbuffer_push_back(CircularBuffer* buffer, u32 data) {
  u32 freespace = cbuffer_freespace(buffer);
  if (freespace < 1) {
    return -1;
  }
  buffer->data[buffer->tail] = data;
  buffer->tail = ((u32)(buffer->tail + 1)) % IO_BUFFER_SIZE;
  return 0;
}

u32 cbuffer_read(const CircularBuffer* buffer, u32* output, u32 nwords) {
  u32 words_to_read = min(nwords, cbuffer_size(buffer));
  u32 tail_words_to_read = min(words_to_read, IO_BUFFER_SIZE - buffer->pos);
  memcpy(
      output,
      &(buffer->data[buffer->pos]),
      sizeof(u32) * tail_words_to_read);
  // check if we need to wrap around.
  u32 remaining_words_at_head = words_to_read - tail_words_to_read;
  if (remaining_words_at_head) {
    memcpy(
        &(output[tail_words_to_read]), 
        buffer->data, 
        sizeof(u32) * remaining_words_at_head);
  }
  return words_to_read;
}

u32 cbuffer_deletefront(CircularBuffer* buffer, u32 nwords) {
  u32 words_to_delete = min(nwords, cbuffer_size(buffer));
  buffer->pos += words_to_delete;
  buffer->pos %= IO_BUFFER_SIZE;
  return words_to_delete;
}

Buffer* cbuffer_pop(CircularBuffer* buffer, u32 nwords) {
  Buffer* output = buffer_new(NULL, nwords);
  u32 actually_read = cbuffer_read(buffer, output->data, nwords);
  buffer_resize(output, actually_read);
  cbuffer_deletefront(buffer, nwords);
  return output;
}
