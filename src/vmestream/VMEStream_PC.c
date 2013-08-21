#include "VMEStream.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

VMEStream *vmestream_initialize(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t MAXRAM)
{
    VMEStream *stream = malloc(sizeof(VMEStream));

    stream->tx_size = malloc(sizeof(uint32_t));
    stream->rx_size = malloc(sizeof(uint32_t));
    *(stream->tx_size) = 0;
    *(stream->rx_size) = 0;

    stream->tx_data = malloc(sizeof(uint32_t));
    stream->rx_data = malloc(sizeof(uint32_t));

    stream->MAXRAM = MAXRAM;

    stream->input = input;
    stream->output = output;

    return stream;
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
    uint32_t data2transfer = min(input_size, MAXSIZE-tx_size);

    if (data2transfer > 0) {
        Buffer *read_data = cbuffer_pop(stream->input, data2transfer);
        memcpy(stream->tx_data, read_data->data,
                data2transfer*sizeof(uint32_t));
        *(stream->tx_size) += data2transfer;

        buffer_free(read_data);
    }

    // ------------------------------------
    // Transfer data from rx_data to output
    // ------------------------------------

    uint32_t output_space = cbuffer_freespace(stream->output);
    uint32_t rx_size      = *(stream->rx_size);

    // number of words to transmit to output
    data2transfer = min(output_space, rx_size);

    if (data2transfer > 0) {
        cbuffer_append(stream->output, stream->rx_data, data2transfer);
        *(stream->rx_size) -= data2transfer;
    }

    return 0;
}
