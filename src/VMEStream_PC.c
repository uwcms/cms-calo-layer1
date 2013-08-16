#include "VMEStream.h"


VMEStream *vmestream_initialize(CircularBuffer *input, CircularBuffer *output)
{
    VMEStream *stream = malloc(sizeof(VMEStream));

    stream->tx_size = malloc(sizeof(uint32_t));
    *(stream->tx_size) = 0;
    stream->tx_data = malloc(sizeof(uint32_t));

    stream->rx_size = malloc(sizeof(uint32_t));
    *(stream->rx_size) = 0;
    stream->rx_data = malloc(sizeof(uint32_t));

    stream->input = input;
    stream->output = output;

    return stream;
}

int vmestream_transfer_data(VMEStream *stream)
{
    if (!stream) {
        return -1;
    }

    uint32_t rx_size = *(stream->rx_size);
    uint32_t tx_size = cbuffer_size(stream->input);

    if (tx_size > 0) {
        *(stream->tx_size) = tx_size;
        *(stream->tx_data) = cbuffer_pop_front(stream->input);
    }

    if (rx_size > 0) {
        if (cbuffer_push_back(stream->output, *(stream->rx_size)) < 0) {
            return -1;
        }
        *(stream->rx_size) = 0;
    }

    return 0;
}
