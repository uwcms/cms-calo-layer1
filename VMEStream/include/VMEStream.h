#ifndef _VMEStream_h
#define _VMEStream_h


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "circular_buffer.h"


typedef struct {
    uint32_t *tx_size;          // Number of words in transmit buffer
    uint32_t *tx_data;          // Transmit buffer
    uint32_t *rx_size;          // Number of words in recieve buffer
    uint32_t *rx_data;          // Recieve buffer
    uint32_t MAXRAM;            // The maximum size (in words) of the VME RAM
    CircularBuffer *input;
    CircularBuffer *output;
} VMEStream;

// Initialize a VMEStream object, allocating transfer/size buffers on the heap
VMEStream *vmestream_initialize_heap(
        CircularBuffer *intput,
        CircularBuffer *output,
        uint32_t MAXRAM);

// Initialize a VMEStream object pointing to existing buffers 
VMEStream *vmestream_initialize_mem(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t *tx_size,
        uint32_t *rx_size,
        uint32_t *tx_data,
        uint32_t *rx_data,
        uint32_t MAXRAM);

// Free memory allocated by the vmestream_initialize_heap function.
void vmestream_destroy_heap(VMEStream *stream);

// Swap data from the circular buffers to the VME transfer RAMs, according to
// the VMEStream protocol. Calling this will move (if possible) data from
// stream->input into stream->tx_data, and (if possible) from stream->rx_data
// to stream->output.
int vmestream_transfer_data(VMEStream *stream);

void do_vme_transfer(VMEStream *stream);

#endif
