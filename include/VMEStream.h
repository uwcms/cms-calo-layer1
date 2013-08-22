#ifndef _VMEStream_h
#define _VMEStream_h


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "circular_buffer.h"


typedef struct {
    uint32_t *tx_size;
    uint32_t *tx_data;
    uint32_t *rx_size;
    uint32_t *rx_data;
    uint32_t MAXRAM;
    CircularBuffer *input;
    CircularBuffer *output;
} VMEStream;

VMEStream *vmestream_initialize(
        CircularBuffer *intput,
        CircularBuffer *output,
        uint32_t MAXRAM);

void vmestream_destroy(VMEStream *stream);

int vmestream_transfer_data(VMEStream *stream);

void do_vme_transfer(VMEStream *stream);


#endif
