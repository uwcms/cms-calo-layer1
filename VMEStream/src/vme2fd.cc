/**
 * VME PC to oRSC Server
 *
 * Streams data between named pipes on the VME PC and the oRSC in the VME
 * crate. There are two named pipes: one for accepting data to send to the oRSC
 * and another to recieve data from the oRSC. An external program simply reads
 * and writes to these pipes to communicate with the oRSC.
 *
 * File: vme2fd.cc
 *
 * Authors: D. Austin Belknap, UW-Madison
 *          Evan K. Friis, UW-Madison
 */
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
#include "VMEStreamAddress.h"
#include "bytebuffer.h"
#include "circular_buffer.h"

#define DATAWIDTH 2

// read 2MB at time
#define READ_BUFFER_SIZE (1024 * 1024 * 2)

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )


/**
 * Read data from VME and load it into a circular buffer.
 *
 * Return the number of 32-bit words read
 */
uint32_t vme_read(VMEController* vme, CircularBuffer* cbuf)
{
    // read ORSC_2_PC_SIZE to determine if there is data to read
    uint32_t rx_size;
    vme->read(ORSC_2_PC_SIZE, DATAWIDTH, &rx_size);

    uint32_t* rx_data = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    if (rx_size > 0 && rx_size <= cbuffer_freespace(cbuf)) {
        vme->block_read(ORSC_2_PC_DATA, DATAWIDTH, rx_data,
                rx_size * sizeof(uint32_t));
        cbuffer_append(cbuf, rx_data, rx_size);

        // write zero to ORSC_2_PC_SIZE to indicate that all data has been read
        uint32_t zero = 0;
        vme->write(ORSC_2_PC_SIZE, DATAWIDTH, &zero);

        free(rx_data);
        return rx_size;
    }

    free(rx_data);
    return 0;
}


/**
 * Write data from a circular buffer to VME.
 *
 * Return the number of 32-bit words written.
 */
uint32_t vme_write(VMEController* vme, CircularBuffer* cbuf)
{
    // read PC_2_ORSC_SIZE to determine if the oRSC is ready to receive more
    // data.
    uint32_t tx_size;
    vme->read(PC_2_ORSC_SIZE, DATAWIDTH, &tx_size);

    uint32_t* tx_data = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    if (tx_size == 0 && cbuffer_size(cbuf) > 0) {
        uint32_t data2transfer = MIN(VMERAMSIZE, cbuffer_size(cbuf));
        Buffer *buf = cbuffer_pop(cbuf, data2transfer);
        memcpy(tx_data, buf->data, data2transfer * sizeof(uint32_t));

        vme->block_write(PC_2_ORSC_DATA, DATAWIDTH, tx_data,
                data2transfer * sizeof(uint32_t));

        // indicate how much data was written to VME RAM
        tx_size = data2transfer;
        vme->write(PC_2_ORSC_SIZE, DATAWIDTH, &tx_size);

        buffer_free(buf);
        free(tx_data);
        return data2transfer;
    }

    free(tx_data);
    return 0;
}


/**
 * Continually loop, reading in data from the input pipe, writting data to VME,
 * and reading data from VME to the output pipe.
 *
 * Acceps the file names of the named pipes as arguments.
 */
void pc2orsc_server(char* in_pipe, char* out_pipe)
{
    // empty buffer.
    ByteBuffer buf = bytebuffer_ctor(NULL, 0);

    int fin  = open(in_pipe, O_RDONLY | O_NONBLOCK);
    int fout = open(out_pipe, O_WRONLY);

    CircularBuffer *input = cbuffer_new();
    CircularBuffer *output = cbuffer_new();

    VMEController* vme = VMEController::getVMEController();

    // Initialize RAM size registers to zero
    uint32_t zero = 0;
    vme->write(PC_2_ORSC_SIZE, DATAWIDTH, &zero);
    vme->write(ORSC_2_PC_SIZE, DATAWIDTH, &zero);

    while (1) {
        // the size of the read buffer will be dynamically resized as necessary
        bytebuffer_read_fd(&buf, fin, READ_BUFFER_SIZE);

        uint32_t words_to_append = MIN(
            buf.bufsize/sizeof(uint32_t), cbuffer_freespace(input));

        // read as much data as possible from the pipe to the input cbuffer
        if (words_to_append > 0) {
            cbuffer_append(input, buf.buf, words_to_append);
        }

        // pop off read words, leaving any fractional words in the input buffer
        bytebuffer_del_front(&buf, words_to_append * sizeof(uint32_t));

        // perform read/write operations to VME
        uint32_t words_read = vme_read(vme, output);
        uint32_t words_write = vme_write(vme, input);

        // if 'ouput' has data, then write it to the output pipe
        uint32_t n_words = cbuffer_size(output);
        if (n_words > 0) {
            cbuffer_write_fd(output, fout, n_words);
        }

        // Do any desired emulation. In production, this does nothing.
        vme->doStuff();
    }

    cbuffer_free(input);
    cbuffer_free(output);

    //delete vme;

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
