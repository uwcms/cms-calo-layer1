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


uint32_t vme_read(VMEController* vme, CircularBuffer* cbuf)
{
    uint32_t rx_size;
    vme->read(ORSC_2_PC_SIZE, DATAWIDTH, &rx_size);

    uint32_t* rx_data = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    if (rx_size > 0 && rx_size <= cbuffer_freespace(cbuf)) {
        vme->block_read(ORSC_2_PC_DATA, DATAWIDTH, rx_data, rx_size * sizeof(uint32_t));
        cbuffer_append(cbuf, rx_data, rx_size);

        uint32_t zero = 0;
        vme->write(ORSC_2_PC_SIZE, DATAWIDTH, &zero);

        free(rx_data);

        return rx_size;
    }

    return 0;
}


uint32_t vme_write(VMEController* vme, CircularBuffer* cbuf)
{
    uint32_t tx_size;
    vme->read(PC_2_ORSC_SIZE, DATAWIDTH, &tx_size);

    uint32_t* tx_data = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    if (tx_size == 0 && cbuffer_size(cbuf) > 0) {
        uint32_t data2transfer = MIN(VMERAMSIZE, cbuffer_size(cbuf));
        Buffer *buf = cbuffer_pop(cbuf, data2transfer);
        memcpy(tx_data, buf->data, data2transfer * sizeof(uint32_t));

        vme->block_write(PC_2_ORSC_DATA, DATAWIDTH, tx_data, data2transfer * sizeof(uint32_t));

        tx_size = data2transfer;
        vme->write(PC_2_ORSC_SIZE, DATAWIDTH, &tx_size);

        buffer_free(buf);
        free(tx_data);

        return data2transfer;
    }

    return 0;
}


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

    VMEController* vme = VMEController::getVMEController();

    // Initialize RAM size registers to zero
    uint32_t zero = 0;
    vme->write(PC_2_ORSC_SIZE, DATAWIDTH, &zero);
    vme->write(ORSC_2_PC_SIZE, DATAWIDTH, &zero);

    while (1) {
        // the size of the read buffer will be dynamically resized
        // as necessary.
        bytebuffer_read_fd(&buf, fin, READ_BUFFER_SIZE);

        uint32_t words_to_append = MIN(
            buf.bufsize/sizeof(uint32_t), cbuffer_freespace(stream->input));

        if (words_to_append > 0) {
            cbuffer_append(stream->input, buf.buf, words_to_append);
        }

        // pop off read words, leaving any fractional words in the input buffer
        bytebuffer_del_front(&buf, words_to_append * sizeof(uint32_t));

        // perform read/write operations to VME
        uint32_t words_read = vme_read(vme, output);
        uint32_t words_write = vme_write(vme, input);

        // if stream->ouput has data, then output it
        uint32_t n_words = cbuffer_size(stream->output);
        if (n_words > 0) {
            cbuffer_write_fd(stream->output, fout, n_words);
        }
        // Do any desired emulation. In production, this does nothing.
        vme->doStuff();
    }

    close(fin);
    close(fout);
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
