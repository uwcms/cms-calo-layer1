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
  output->size = 0;
  output->pos = 0;
  return output;
}

void cbuffer_free(CircularBuffer* tokill) {
  free(tokill->data);
  free(tokill);
}

Buffer* buffer_new(void* data, u16 size) {
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

int cbuffer_freespace(CircularBuffer* buf) {
  return IO_BUFFER_SIZE - buf->size;
}

int cbuffer_append(CircularBuffer* buffer, void* data, u16 nwords) {
  int freespace = cbuffer_freespace(buffer);
  if (freespace < nwords) {
    return -1;
  }
  int tail_pos = (buffer->pos + buffer->size) % IO_BUFFER_SIZE;

  // bytes available on "tail" until we hit the absolute end of the memory.
  // we know from the freespace check that we can't overwrite the head with
  // only <nwords>.
  int tail_length = IO_BUFFER_SIZE - tail_pos;

  memcpy(
      &(buffer->data[tail_pos]),
      data, 
      sizeof(u32) * min(nwords, tail_length));

  // if we didn't write everything, we need to wrap around to the beginning.
  if (tail_length < nwords) {
    memcpy(
        buffer->data, 
        data + sizeof(u32) * tail_length,
        sizeof(u32) * (nwords - tail_length));
  }
  buffer->size += nwords;
  return 0;
}

Buffer* cbuffer_read(CircularBuffer* buffer, u16 nwords) {
  int words_to_read = min(nwords, buffer->size);
  Buffer* output = buffer_new(0, words_to_read);
  int tail_words_to_read = min(words_to_read, IO_BUFFER_SIZE - buffer->pos);
  memcpy(
      output->data, 
      &(buffer->data[buffer->pos]),
      sizeof(u32) * tail_words_to_read);
  // check if we need to wrap around.
  int remaining_words_at_head = words_to_read - tail_words_to_read;
  if (remaining_words_at_head) {
    memcpy(
        &(output->data[tail_words_to_read]), 
        buffer->data, 
        sizeof(u32) * remaining_words_at_head);
  }
  return output;
}

int cbuffer_deletefront(CircularBuffer* buffer, u16 nwords) {
  int words_to_delete = min(nwords, buffer->size);
  buffer->size -= words_to_delete;
  buffer->pos += words_to_delete;
  buffer->pos %= IO_BUFFER_SIZE;
  return words_to_delete;
}

Buffer* cbuffer_pop(CircularBuffer* buffer, u16 nwords) {
  Buffer* output = cbuffer_read(buffer, nwords);
  cbuffer_deletefront(buffer, nwords);
  return output;
}
