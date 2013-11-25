/*
 * Minimal test of VMEStream communication on the oRSC frontend.
 *
 * Authors: Evan K. Friis, UW Madison
 *          D. Austin Belknap, UW Madison
 *
 * Accepts data from VME and reflects it back to VME to be read
 * by vme2fd running on the VME PC.
 */

#include "platform.h"

#include "xparameters.h"        /* Defined in BSP */
#include "xio.h"

#include "VMEStreamAddress_ORSC.h"
#include "circular_buffer.h"
#include "string.h"

#include "xil_printf.h"

/* Input and output buffers */
static CircularBuffer* input;
static CircularBuffer* output;

#define MEMSTEP 4

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )


uint32_t vme_read(CircularBuffer* cbuf)
{
    // check if there is data in VME RAM to read
    uint16_t rx_size = XIo_In16(PC_2_ORSC_SIZE);

    uint32_t rx_data [VMERAMSIZE];

    if (rx_size > 0 && rx_size <= cbuffer_freespace(cbuf)) {
        for (uint32_t i = 0; i < VMERAMSIZE * 2; ++i) {
            ((uint16_t*)rx_data)[i] = XIo_In16(PC_2_ORSC_DATA + i*MEMSTEP);
        }
        cbuffer_append(cbuf, rx_data, (uint32_t)rx_size);

        XIo_Out16(PC_2_ORSC_SIZE, (uint16_t)0);

        return (uint32_t)rx_size;
    }
    return 0;
}


uint32_t vme_write(CircularBuffer* cbuf)
{
    // check if VME PC is ready to accept more data
    uint16_t tx_size = XIo_In16(ORSC_2_PC_SIZE);

    uint32_t tx_data [VMERAMSIZE];

    if (tx_size == 0 && cbuffer_size(cbuf) > 0) {
        uint32_t data2transfer = MIN(VMERAMSIZE, cbuffer_size(cbuf));
        Buffer *buf = cbuffer_pop(cbuf, data2transfer);
        memcpy(tx_data, buf->data, data2transfer * sizeof(uint32_t));

        for (uint32_t i = 0; i < VMERAMSIZE * 2; ++i) {
            XIo_Out16(ORSC_2_PC_DATA + i*MEMSTEP, ((uint16_t*)tx_data)[i]);
        }

        tx_size = (uint16_t)data2transfer;
        XIo_Out16(ORSC_2_PC_SIZE, tx_size);

        buffer_free(buf);

        return data2transfer;
    }
    return 0;
}


void orsc2pc_server()
{
    input = cbuffer_new();
    output = cbuffer_new();

    while (1) {
        uint32_t words_read = vme_read(output);
        uint32_t words_write = vme_write(input);

        // move data from 'output' to 'input' so it will be sent
        // back to vme2fd across VME.
        while (cbuffer_size(output) && cbuffer_freespace(input)) {
            uint32_t word = cbuffer_pop_front(output);
            xil_printf("%s", (char*)(&word));
            cbuffer_push_back(input, word);
        }
    }
}


int main()
{
    // initialize stdout.
    init_platform();
    xil_printf("VMEStream - oRSC echo test\r\n");

    xil_printf("Start Server\r\n");

    orsc2pc_server();

    return 0;
}

