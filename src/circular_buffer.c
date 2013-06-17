#include <stdlib.h>
#include <string.h>

#include "circular_buffer.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

CircularBuffer* cbuffer_new(void) {
  CircularBuffer* output = malloc(sizeof(CircularBuffer));
  output->data = malloc(IO_BUFFER_SIZE * sizeof(u8));
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
  output->data = malloc(size * sizeof(u8));
  if (data) {
    memcpy(output->data, data, size);
  } else {
    memset(output->data, 0, size);
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

int cbuffer_append(CircularBuffer* buffer, void* data, u16 nbytes) {
  int freespace = cbuffer_freespace(buffer);
  if (freespace < nbytes) {
    return -1;
  }
  int tail_pos = (buffer->pos + buffer->size) % IO_BUFFER_SIZE;

  // bytes available on "tail" until we hit the absolute end of the memory.
  // we know from the freespace check that we can't overwrite the head with
  // only <nbytes>.
  int tail_length = IO_BUFFER_SIZE - tail_pos;

  memcpy(buffer->data + tail_pos, data, min(nbytes, tail_length));

  // if we didn't write everything, we need to wrap around to the beginning.
  if (tail_length < nbytes) {
    memcpy(buffer->data, data + tail_length, nbytes - tail_length);
  }
  return 0;
}

Buffer* cbuffer_read(CircularBuffer* buffer, u16 nbytes) {
  int bytes_to_read = min(nbytes, buffer->size);
  Buffer* output = buffer_new(0, bytes_to_read);
  int tail_bytes_to_read = min(bytes_to_read, IO_BUFFER_SIZE - buffer->pos);
  memcpy(output->data, buffer->data + buffer->pos, tail_bytes_to_read);
  // check if we need to wrap around.
  int remaining_bytes_at_head = bytes_to_read - tail_bytes_to_read;
  if (remaining_bytes_at_head) {
    memcpy(output->data + tail_bytes_to_read, buffer->data, 
        remaining_bytes_at_head);
  }
  return output;
}

int cbuffer_deletefront(CircularBuffer* buffer, u16 nbytes) {
  int bytes_to_delete = min(nbytes, buffer->size);
  buffer->size -= bytes_to_delete;
  buffer->pos += bytes_to_delete;
  buffer->pos %= IO_BUFFER_SIZE;
  return bytes_to_delete;
}
