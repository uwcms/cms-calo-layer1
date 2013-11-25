/*
 * Minimal test of VMEStream communication on the oRSC frontend.
 *
 * Authors: Evan K. Friis, UW Madison
 *          D. Austin Belknap, UW Madison
 *
 * This program sets up the SPI as master, then sends a continuous stream of
 * increasing words to the slave device, which should echo them back.  If there
 * are transmission errors, they will be noted on stdout.
 * 
 */

#include "platform.h"

#include "xparameters.h"        /* Defined in BSP */
#include "xio.h"

#include "VMEStream.h"
#include "VMEStreamAddress_ORSC.h"

#include "xil_printf.h"

/* Input and output buffers */
static CircularBuffer* input;
static CircularBuffer* output;

#define MEMSTEP 4

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )


uint32_t vme_read(CircularBuffer* cbuf)
{
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

        return data2transfer;
    }
    return 0;
}


int main() {
  // initialize stdout.
  init_platform();
  xil_printf("VMEStream - oRSC echo test\r\n");

  input = cbuffer_new();
  output = cbuffer_new();

  xil_printf("Start Server\r\n");
  while (1) {
    uint32_t words_read = vme_read(output);
    uint32_t words_write = vme_write(input);

    while (cbuffer_size(output) && cbuffer_freespace(input)) {
      uint32_t word = cbuffer_pop_front(output);
      xil_printf("%s", (char*)(&word));
      cbuffer_push_back(input, word);
    }


    //// transfer data
    //vmestream_transfer_data(stream);
    //// now echo the data
    //while (cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)) {
    //    uint32_t word = cbuffer_pop_front(rx_buffer);
    //    xil_printf("word: %x\r\n", word);
    //    cbuffer_push_back(tx_buffer, cbuffer_pop_front(rx_buffer));
    //}
  }

  return 0;
}

