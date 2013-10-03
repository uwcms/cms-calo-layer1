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
#include "bytebuffer.h"


#define VME_RX_SIZE_ADDR 0xDEADBEEF
#define VME_TX_SIZE_ADDR 0xBEEFCAFE

#define VME_RX_DATA_ADDR 0xFACEFEED
#define VME_TX_DATA_ADDR 0xCAFEBABE

#define DATAWIDTH 4

// read 2MB at time
#define READ_BUFFER_SIZE (1024 * 1024 * 2)

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )

// VME RAM holds 512 words
const uint32_t MAXRAM = 512;


int main ( int argc, char** argv )
{
    int fin;
    int fout;

    // empty buffer.
    ByteBuffer buf = bytebuffer_ctor(NULL, 0);

    if ( argc != 3 ) {
        printf("Usage: vme2fd [instream] [outstream]\n");
        exit(0);
    }
    fin  = open(argv[1], O_RDONLY);
    fout = open(argv[2], O_WRONLY);

    CircularBuffer *input = cbuffer_new();
    CircularBuffer *output = cbuffer_new();
    // initialize VMEStream object allocating local memory for the transfer
    // buffers.
    VMEStream *stream = vmestream_initialize_heap(input, output, MAXRAM);

    uint32_t vme_tx_size;
    uint32_t vme_rx_size;

    VMEController* vme = VMEController::getVMEController();

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

        vmestream_transfer_data(stream);

        vme->read(VME_TX_SIZE_ADDR, DATAWIDTH, &vme_tx_size);
        if (vme_tx_size == 0 && *(stream->tx_size) > 0) {
            /* vme->write(VME_TX_DATA_ADDR, DATAWIDTH, stream->tx_data); */
            vme->write(VME_TX_DATA_ADDR, *(stream->tx_size), stream->tx_data);
            vme->write(VME_TX_SIZE_ADDR, DATAWIDTH, stream->tx_size);
            *(stream->tx_size) = 0;
        }

        vme->read(VME_RX_SIZE_ADDR, DATAWIDTH, &vme_rx_size);
        if (vme_rx_size > 0 && *(stream->rx_size) == 0) {
            /* vme->read(VME_RX_DATA_ADDR, DATAWIDTH, stream->rx_data); */
            vme->read(VME_RX_DATA_ADDR, vme_rx_size, stream->rx_data);
            *(stream->rx_size) = vme_rx_size;
            uint32_t zero = 0;
            vme->write(VME_RX_SIZE_ADDR, DATAWIDTH, &zero);
        }

        vmestream_transfer_data(stream);

        // if stream->ouput has data, then output it
        uint32_t n_words = cbuffer_size(stream->output);
        if (n_words > 0) {
            cbuffer_write_fd(stream->output, fout, n_words);
        }
        // Do any desired emulation. In production, this does nothing.
        vme->doStuff();
    }

    close( fin );
    close( fout );
    vmestream_destroy_heap(stream);

    return 0;
}
