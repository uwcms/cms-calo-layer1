/*
 * =====================================================================================
 *
 *       Filename:  spi_stream.c
 *
 *    Description:  Tests of SPI stream protocol.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>

#include "minunit.h"
#include "spi_stream.h"

// These buffer pointers + call backs simulate the behavior of the SPI driver
// on each device.
static u32* master_tx_fifo;
static u32* master_rx_fifo;
static u16 master_nbytes;
static u32* slave_tx_fifo;
static u32* slave_rx_fifo;
static u16 slave_nbytes;

static void master_transmit_callback(u8* tx, u8* rx, u16 nbytes) {
  master_tx_fifo = (u32*)tx;
  master_rx_fifo = (u32*)rx;
  master_nbytes = nbytes;
}
static void slave_transmit_callback(u8* tx, u8* rx, u16 nbytes) {
  slave_tx_fifo = (u32*)tx;
  slave_rx_fifo = (u32*)rx;
  slave_nbytes = nbytes;
}

// Run a simulation of data exchange between two devices
// Returns number of SPI transfers before data was exhausted on both ends.
// Returns -1 if there was an un-recovered data error
int simulate_interaction(
    CircularBuffer* master_data,
    CircularBuffer* slave_data,
    // array of transmission indices to screw up the master->slave data packet
    u16* master_errors,
    u16 n_master_errors,
    // simulate slave->master data packet errors
    u16* slave_errors,
    u16 n_slave_errors) {

  // clobberable copy of the test data we send
  CircularBuffer* master_tx = cbuffer_copy(master_data);
  CircularBuffer* slave_tx = cbuffer_copy(slave_data);

  // where the "received" data goes
  CircularBuffer* master_rx = cbuffer_new();
  CircularBuffer* slave_rx = cbuffer_new();

  SPIStream* master_spi = spi_stream_init(
      master_tx, master_rx, master_transmit_callback);
  SPIStream* slave_spi = spi_stream_init(
      slave_tx, slave_rx, slave_transmit_callback);

  int n_transfers = 0;
  while (cbuffer_size(master_tx) || cbuffer_size(slave_tx)) {

    // check if we want to simulate mangling this data
    int master_error = 0;
    for (int i = 0; i < n_master_errors; ++i) {
      if (i == n_transfers)
        master_error = 1;
    }
    int slave_error = 0;
    for (int i = 0; i < n_slave_errors; ++i) {
      if (i == n_transfers)
        slave_error = 1;
    }

    spi_stream_transfer_data(master_spi, 0);
    spi_stream_transfer_data(slave_spi, 0);
    // emulate HW + driver 
    memcpy(master_rx_fifo, slave_tx_fifo, SPI_BUFFER_SIZE * sizeof(u32));
    memcpy(slave_rx_fifo, master_tx_fifo, SPI_BUFFER_SIZE * sizeof(u32));

    // emulate errors if desired - 10 & 20 are just random numbers, could be
    // anything.
    if (master_error) 
      master_rx_fifo[10]++;
    if (slave_error) 
      slave_rx_fifo[20]++;

    n_transfers++;
  }
  // check if we transferred the data correction
  if (cbuffer_size(master_data) != cbuffer_size(master_rx)) {
    return -1;
  }
  if (cbuffer_size(slave_data) != cbuffer_size(slave_rx)) {
    return -2;
  }
  for (int i = 0; i < cbuffer_size(master_data); ++i) {
    if (cbuffer_value_at(master_data, i) != cbuffer_value_at(master_rx, i)) {
      return -3;
    }
  }
  for (int i = 0; i < cbuffer_size(slave_data); ++i) {
    if (cbuffer_value_at(slave_data, i) != cbuffer_value_at(slave_rx, i)) {
      return -4;
    }
  }
  return n_transfers;
}


int tests_run;

char * all_tests(void) {
  printf("\n\n=== spi_stream tests ===\n");
  return 0;
}
