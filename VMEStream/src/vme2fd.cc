#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>

#include "VMEController.h"
#include "VMEStream.h"
#include "VMEStreamAddress.h"
#include "bytebuffer.h"

#define DATAWIDTH 2

// read 2MB at time
#define READ_BUFFER_SIZE (1024 * 1024 * 2)

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )


void pc2orsc_server(char* in_pipe, char* out_pipe)
{
    // empty buffer.
    ByteBuffer buf = bytebuffer_ctor(NULL, 0);

    int fin  = open(in_pipe, O_RDONLY);
    int fout = open(out_pipe, O_WRONLY);

    CircularBuffer *input = cbuffer_new();
    CircularBuffer *output = cbuffer_new();
    // initialize VMEStream object allocating local memory for the transfer
    // buffers.
    VMEStream *stream = vmestream_initialize_heap(input, output, VMERAMSIZE);

    VMEController* vme = VMEController::getVMEController();

    while (1) {
        bytebuffer_read_fd(&buf, fin, READ_BUFFER_SIZE);

        uint32_t words2append = MIN(buf.bufsize/sizeof(uint32_t), cbuffer_freespace(stream->input));

        if (words2append > 0) {
            cbuffer_append(stream->input, buf.buf, words2append);
        }
        bytebuffer_del_front(&buf, words2append * sizeof(uint32_t));

        // update flags w/ VME
	//         vme->read(ORSC_RECV_SIZE, DATAWIDTH, stream->remote_recv_size);
	//         vme->read(ORSC_SEND_SIZE, DATAWIDTH, stream->remote_send_size);
	//         vme->write(PC_RECV_SIZE, DATAWIDTH, stream->local_recv_size);
	//         vme->write(PC_SEND_SIZE, DATAWIDTH, stream->local_send_size);

        // recieve data via VME
        if (*(stream->remote_send_size) > 0) {
            vme->block_read(ORSC2PC_DATA, DATAWIDTH, stream->recv_data, *(stream->remote_send_size) * sizeof(uint32_t));
        }

        // move data in/out of the buffers and update size values
        vmestream_transfer_data(stream);

        // send data via VME
        if (*(stream->local_send_size) > 0) {
            vme->block_write(PC2ORSC_DATA, DATAWIDTH, stream->send_data, *(stream->local_send_size) * sizeof(uint32_t));
        }

        uint32_t recv_words = cbuffer_size(stream->output);
        if (recv_words > 0) {
            cbuffer_write_fd(stream->output, fout, recv_words);
        }

        vme->doStuff();
    }

    close(fin);
    close(fout);
    vmestream_destroy_heap(stream);
}


int main (int argc, char** argv)
{
    if ( argc != 3 ) {
        printf("Usage: vme2fd [instream] [outstream]\n");
        exit(1);
    }

    pc2orsc_server(argv[1], argv[2]);

    return 0;
}
