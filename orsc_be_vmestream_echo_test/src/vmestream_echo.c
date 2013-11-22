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

static VMEStream* stream;
/* Input and output buffers */
static CircularBuffer* input;
static CircularBuffer* output;

#define MEMSTEP 4


int main() {
  // initialize stdout.
  init_platform();
  xil_printf("VMEStream - oRSC echo test\r\n");

  input = cbuffer_new();
  output = cbuffer_new();

  stream = vmestream_initialize_heap(input, output, VMERAMSIZE);

  uint16_t tx_size;
  uint16_t rx_size;
  uint16_t* tx_data = (uint16_t*) calloc(VMERAMSIZE, sizeof(uint32_t));
  uint16_t* rx_data = (uint16_t*) calloc(VMERAMSIZE, sizeof(uint32_t));

  xil_printf("Start Server\r\n");
  while (1) {
    rx_size = XIo_In16(PC_2_ORSC_SIZE);
    tx_size = XIo_In16(ORSC_2_PC_SIZE);

    for (uint32_t i = 0; i < VMERAMSIZE * 2; ++i) {
      rx_data[i] = XIo_In16(PC_2_ORSC_DATA + i*MEMSTEP);
    }

    if (rx_size) {
        xil_printf("Transfering data\r\n");
        memcpy(tx_data, rx_data, rx_size * sizeof(uint32_t));
        for (uint32_t i = 0; i < VMERAMSIZE * 2; ++i) {
            XIo_Out16(ORSC_2_PC_DATA + i*MEMSTEP, tx_data[i]);
        }
        tx_size = rx_size;
        rx_size = 0;
        XIo_Out16(ORSC_2_PC_SIZE, tx_size);
        XIo_Out16(PC_2_ORSC_SIZE, rx_size);
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

