/*
 * Minimal test of VMEStream communication on the oRSC frontend.
 *
 * Author: Evan K. Friis, UW Madison
 *
 * This program sets up the SPI as master, then sends a continuous stream of
 * increasing words to the slave device, which should echo them back.  If there
 * are transmission errors, they will be noted on stdout.
 * 
 */

#include "platform.h"

#include "xparameters.h"        /* Defined in BSP */

#include "VMEStream.h"
#include "VMEStreamAddress.h"

#include "xil_printf.h"

static VMEStream* vme_stream;
/* Input and output buffers */
static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;

int main() {
  xil_printf("Master SPI oRSC echo test\n");
  // initialize stdout.
  init_platform();

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  vme_stream = vmestream_initialize_mem(
          rx_buffer, tx_buffer, 
          (uint32_t*)ORSC_2_PC_SIZE,
          (uint32_t*)PC_2_ORSC_SIZE,
          (uint32_t*)ORSC_2_PC_DATA,
          (uint32_t*)PC_2_ORSC_DATA,
          VMERAMSIZE);

  //printf("Master SPI oRSC echo test\n");

  while (1) {
      // transfer data
      vmestream_transfer_data(vme_stream);
      // now echo the data
      while (cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)) {
          cbuffer_push_back(tx_buffer, cbuffer_pop_front(rx_buffer));
      }
  }

  return 0;
}

