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
    VMEStream *stream = (VMEStream*)malloc(sizeof(VMEStream));

    stream->local_send_size = 0;
    stream->local_recv_size = 0;
    stream->remote_send_size = 0;
    stream->remote_recv_size = 0;

    stream->recv_data = (uint32_t*)calloc(MAXRAM, sizeof(uint32_t));
    stream->send_data = (uint32_t*)calloc(MAXRAM, sizeof(uint32_t));

    stream->MAXRAM = MAXRAM;

    stream->input = input;
    stream->output = output;

    return stream;
}


VMEStream *vmestream_initialize_mem(
        CircularBuffer *input,
        CircularBuffer *output,
        uint32_t local_send_size,
        uint32_t local_recv_size,
        uint32_t remote_send_size,
        uint32_t remote_recv_size,
        uint32_t* recv_data,
        uint32_t* send_data,
        uint32_t MAXRAM) {
    VMEStream *stream = (VMEStream*)malloc(sizeof(VMEStream));

    stream->local_send_size = local_send_size;
    stream->local_recv_size = local_recv_size;
    stream->remote_send_size = remote_send_size;
    stream->remote_recv_size = remote_recv_size;

    stream->recv_data = recv_data;
    stream->send_data = send_data;

    stream->MAXRAM = MAXRAM;

    return stream;
}


void vmestream_destroy_heap(VMEStream *stream)
{
    free(stream->recv_data);
    free(stream->send_data);

    free(stream);
}


int vmestream_transfer_data(VMEStream *stream)
{
    if (!stream) return -1;

    if (!stream->input) return -1;
    if (!stream->output) return -1;

    // -------------------------------------
    // Transfer data from input to send_data
    // -------------------------------------

    uint32_t input_size = cbuffer_size(stream->input);

    uint32_t data2transfer = min(input_size, stream->MAXRAM);

    // check if the remote has read all data sent
    if (stream->local_send_size == stream->remote_recv_size) {
        // if the RAM is empty, we are ready to send more data
        if (stream->local_send_size == 0) {
            Buffer* buffer = cbuffer_pop(stream->input, data2transfer);
            memcpy(stream->send_data, buffer->data, data2transfer * sizeof(uint32_t));
            stream->local_send_size = data2transfer;
        }
        // reset the size counter 0 to prepare for another transfer
        else {
            stream->local_send_size = 0;
        }
    }

    // -------------------------------------
    // Transfer data from input to recv_data
    // -------------------------------------

    // reset size counter to 0 to prepare for another transfer
    if (stream->remote_send_size == 0) {
        stream->local_recv_size = 0;
    }
    // check if there is data to recieve
    else if (stream->remote_send_size > 0 && stream->local_recv_size == 0) {
        // check if there is room in output to recieve the data
        if (cbuffer_freespace(stream->output) >= stream->remote_send_size) {
            cbuffer_append(stream->output, stream->recv_data, stream->remote_send_size);
            stream->local_recv_size = stream->remote_send_size;
        }
    }

    return 0;
}
