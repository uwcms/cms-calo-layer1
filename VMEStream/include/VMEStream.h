#ifndef _VMEStream_h
#define _VMEStream_h


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "circular_buffer.h"


typedef struct {
    uint32_t local_send_size;    // How many words are loaded into send_data
    uint32_t local_recv_size;    // How many words have been read from recv_data
    uint32_t remote_send_size;   // How many words are loaded into recv_data
    uint32_t remote_recv_size;   // How many words are read from send_data

    uint32_t* recv_data;         // Recieve buffer (read only)
    uint32_t* send_data;         // Transmit buffer (write only)
    uint32_t MAXRAM;             // The maximum size (in words) of the VME RAM

    CircularBuffer *input;
    CircularBuffer *output;
} VMEStream;


// Initialize a VMEStream object, allocating transfer/size buffers on the heap
VMEStream *vmestream_initialize_heap(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t MAXRAM);


// Initialize a VMEStream object pointing to existing buffers
VMEStream *vmestream_initialize_mem(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t local_send_size,
        uint32_t local_recv_size,
        uint32_t remote_send_size,
        uint32_t remote_recv_size,
        uint32_t* recv_data,
        uint32_t* send_data,
        uint32_t MAXRAM);
        

// Free memory allocated by the vmestream_initialize_heap function.
void vmestream_destroy_heap(VMEStream *stream);

/** Perform a VMEStream datatransfer
 */
int vmestream_transfer_data(VMEStream *stream);

void do_vme_transfer(VMEStream *stream);

#endif
