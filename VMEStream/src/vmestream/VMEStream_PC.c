#include "VMEStream.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

VMEStream *vmestream_initialize_heap(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t MAXRAM)
{
    VMEStream *stream = (VMEStream*) malloc(sizeof(VMEStream));

    stream->tx_size = (uint32_t*) calloc(1, sizeof(uint32_t));
    stream->rx_size = (uint32_t*) calloc(1, sizeof(uint32_t));

    stream->tx_data = (uint32_t*) calloc(MAXRAM, sizeof(uint32_t));
    stream->rx_data = (uint32_t*) calloc(MAXRAM, sizeof(uint32_t));

    stream->MAXRAM = MAXRAM;

    stream->input = input;
    stream->output = output;

    return stream;
}

VMEStream *vmestream_initialize_mem(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t *tx_size,
        uint32_t *rx_size,
        uint32_t *tx_data,
        uint32_t *rx_data,
        uint32_t MAXRAM) {
    VMEStream *stream = (VMEStream*)malloc(sizeof(VMEStream));
    stream->tx_size = tx_size;
    stream->tx_data = tx_data;
    stream->rx_size = rx_size;
    stream->rx_data = rx_data;
    stream->MAXRAM = MAXRAM;
    stream->input = input;
    stream->output = output;
    return stream;
}

void vmestream_destroy_heap(VMEStream *stream)
{
    free(stream->tx_size);
    free(stream->tx_data);
    free(stream->rx_size);
    free(stream->rx_data);

    free(stream);
}


int vmestream_transfer_data(VMEStream *stream)
{
    if (!stream) return -1;

    if (!stream->input) return -1;
    if (!stream->output) return -1;

    // -----------------------------------
    // Transfer data from input to tx_data
    // -----------------------------------

    uint32_t input_size = cbuffer_size(stream->input);
    uint32_t tx_size    = *(stream->tx_size);
    uint32_t MAXSIZE    = stream->MAXRAM;

    // number of words to transmit to tx_data
    uint32_t data2transfer = min(input_size, MAXSIZE);

    // check if any previously sent data has been received.
    if (tx_size == 0 && data2transfer > 0) {
      Buffer *read_data = cbuffer_pop(stream->input, data2transfer);
      memcpy(stream->tx_data, read_data->data,
          data2transfer*sizeof(uint32_t));
      *(stream->tx_size) = data2transfer;

      buffer_free(read_data);
    }

    // ------------------------------------
    // Transfer data from rx_data to output
    // ------------------------------------

    uint32_t output_space = cbuffer_freespace(stream->output);
    uint32_t rx_size      = *(stream->rx_size);

    // just leave the data in limbo until we have room for it.
    if (rx_size && output_space >= rx_size) {
        cbuffer_append(stream->output, stream->rx_data, rx_size);
        // indicate successful receipt.
        *(stream->rx_size) = 0;
    }

    return 0;
}
