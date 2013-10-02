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

#include "spi_stream.h"

/*  STDOUT functionality  */
void print(char *str);

static SPIStream* spi_stream = NULL;
/* Input and output buffers */
static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;

int main() {
  // initialize stdout.
  init_platform();

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  spi_stream = spi_stream_init(
      tx_buffer, rx_buffer, 
      DoSpiTransfer, // callback which triggers a SPI transfer
      0);

  print("Master SPI oRSC echo test\n");

  while (1) {
    // fill up the transmit buffer
    while (cbuffer_freespace(tx_buffer)) {
      cbuffer_push_back(tx_buffer, current_tx++);
    }
    // check to make sure the received buffer is what we expect
    while (cbuffer_size(rx_buffer)) {
      u32 front = cbuffer_value_at(rx_buffer, 0);
      if (front != expected_rx) {
        //print("Error: expected %lx, got %lx!\n", expected_rx, front);
        print("Error: data value\n");
      }
      expected_rx++;
      cbuffer_deletefront(rx_buffer, 1);
    }
  }

  return 0;
}

