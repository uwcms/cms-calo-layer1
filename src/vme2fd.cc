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


#define VME_RX_SIZE_ADDR 0xDEADBEEF
#define VME_TX_SIZE_ADDR 0xBEEFCAFE

#define VME_RX_DATA_ADDR 0xDEADBEEF
#define VME_TX_DATA_ADDR 0xBEEFCAFE

#define DATAWIDTH 4


const uint32_t MAXRAM = 1;


int main ( int argc, char** argv )
{
    int fin;
    int fout;

    uint32_t buf [512];

    if ( argc != 3 ) {
        printf("Usage: vme2fd [instream] [outstream]\n");
        exit(0);
    }

    fin  = open( argv[1], O_RDONLY );
    fout = open( argv[2], O_WRONLY );

    CircularBuffer *input = cbuffer_new();
    CircularBuffer *output = cbuffer_new();
    VMEStream *stream = vmestream_initialize(input, output, MAXRAM);

    uint32_t vme_tx_size;
    uint32_t vme_rx_size;

    /*
    uint32_t vme_tx_data[MAXRAM];
    uint32_t vme_rx_data[MAXRAM];

    uint32_t vme_tx_address;
    uint32_t vme_rx_address;
    */

    VMEController* vme = VMEController::getVMEController();

    while (1) {
        uint32_t words_free = cbuffer_freespace(stream->input);

        // read only as much as the cbuffer can handle
        ssize_t bytes_read = read(fin, buf, words_free*sizeof(uint32_t));
        uint32_t words_read = bytes_read/sizeof(uint32_t);

        if (words_read > 0) {
            cbuffer_append(stream->input, buf, words_read);
        }

        vmestream_transfer_data(stream);

        vme->read(VME_TX_SIZE_ADDR, DATAWIDTH, &vme_tx_size);
        if (vme_tx_size == 0 && *(stream->tx_size) > 0) {
            vme->write(VME_TX_DATA_ADDR, DATAWIDTH, stream->tx_data);
            vme->write(VME_TX_SIZE_ADDR, DATAWIDTH, stream->tx_size);
            *(stream->tx_size) = 0;
        }

        vme->read(VME_RX_SIZE_ADDR, DATAWIDTH, &vme_rx_size);
        if (vme_rx_size > 0 && *(stream->rx_size) == 0) {
            vme->read(VME_RX_DATA_ADDR, DATAWIDTH, stream->rx_data);
            *(stream->rx_size) = vme_rx_size;
            uint32_t zero = 0;
            vme->write(VME_RX_SIZE_ADDR, DATAWIDTH, &zero);
        }

        vmestream_transfer_data(stream);

        // if stream->ouput has data, then output it
        uint32_t n_words = cbuffer_size(stream->output);
        if (n_words > 0) {
            Buffer *outbuf = cbuffer_pop(stream->output, n_words);
            write(fout, outbuf->data, n_words*sizeof(uint32_t));
            buffer_free(outbuf);
        }
    }

    close( fin );
    close( fout );

    return 0;
}
