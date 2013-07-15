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
static char * simulate_interaction(
    CircularBuffer* master_data,
    CircularBuffer* slave_data,
    // array of transmission indices to screw up the master->slave data packet
    u16* master_errors,
    u16 n_master_errors,
    // simulate slave->master data packet errors
    u16* slave_errors,
    u16 n_slave_errors,
    int *n_transactions) {

  // clobberable copy of the test data we send
  CircularBuffer* master_tx = cbuffer_copy(master_data);
  CircularBuffer* slave_tx = cbuffer_copy(slave_data);

  //printf("start: %li %li\n", cbuffer_size(master_tx), cbuffer_size(slave_tx));

  // where the "received" data goes
  CircularBuffer* master_rx = cbuffer_new();
  CircularBuffer* slave_rx = cbuffer_new();

  SPIStream* master_spi = spi_stream_init(
      master_tx, master_rx, master_transmit_callback);
  SPIStream* slave_spi = spi_stream_init(
      slave_tx, slave_rx, slave_transmit_callback);

  *n_transactions = 0;
  //printf("init:  %li %li\n", cbuffer_size(master_tx), cbuffer_size(slave_tx));

  // Use these to detect changes in receive buffer size
  u32 master_rx_size = 0;
  u32 slave_rx_size = 0;

  while (
      // more data to send
      cbuffer_size(master_tx) || cbuffer_size(slave_tx) 
      // We received on the last exchange.  We need to try at least one more
      // to make sure we don't orphan the last frame.  In the normal case,
      // we never stop transferring so this doesn't matter.
      || cbuffer_size(master_rx) > master_rx_size
      || cbuffer_size(slave_rx) > slave_rx_size) {

    master_rx_size = cbuffer_size(master_rx);
    slave_rx_size = cbuffer_size(slave_rx);
    
    if (*n_transactions < 30) {
      //printf("\niter %i: %li %li\n", *n_transactions, cbuffer_size(master_tx), cbuffer_size(slave_tx));
    } else {
    }

    // check if we want to simulate mangling this data
    int master_error = 0;
    for (int i = 0; i < n_master_errors; ++i) {
      if (master_errors[i] == *n_transactions) {
        printf("master error @ %i\n", i);
        master_error = 1;
      }
    }
    int slave_error = 0;
    for (int i = 0; i < n_slave_errors; ++i) {
      if (slave_errors[i] == *n_transactions) {
        printf("slave error @ %i\n", i);
        slave_error = 1;
      }
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

    (*n_transactions)++;
  }
//  for (int i = 0; i < cbuffer_size(slave_data); ++i) {
//    printf("%i %i %i\n", i, (int)cbuffer_value_at(slave_data, i), (int)cbuffer_value_at(master_rx, i));
//  }
  // check if we transferred the data correction
  mu_assert_eq("master size", cbuffer_size(slave_data), cbuffer_size(master_rx));
  mu_assert_eq("slave size", cbuffer_size(master_data), cbuffer_size(slave_rx));
  for (int i = 0; i < cbuffer_size(master_data); ++i) {
    if (cbuffer_value_at(master_data, i) != cbuffer_value_at(slave_rx, i)) {
      printf("error in master-> @ %i!\n", i);
      mu_assert_eq("content", cbuffer_value_at(master_data, i), cbuffer_value_at(slave_rx, i));
    }
  }
  for (int i = 0; i < cbuffer_size(slave_data); ++i) {
    if (cbuffer_value_at(slave_data, i) != cbuffer_value_at(master_rx, i)) {
      printf("error in slave->master @ %i!\n", i);
      mu_assert_eq("content", cbuffer_value_at(slave_data, i), cbuffer_value_at(master_rx, i));
    }
  }
  return 0;
}

CircularBuffer* make_some_data(u16 size, u16 multiplier) {
  CircularBuffer* output = cbuffer_new();
  for (int i = 0; i < size; ++i) {
    cbuffer_push_back(output, multiplier * i);
  }
  return output;
}

static char* test_no_errors(void) {
  CircularBuffer* master_data = make_some_data(1000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  int transactions; 
  char* res = simulate_interaction(master_data, slave_data, NULL, 0, NULL, 0, &transactions);
  if (res)
    return res;
  // we should need 5000/(512 - 3) = 9.823182711 -> 10 interactions
  // we get one extra at the end, plus the initial 1 = 12
  mu_assert_eq("expected transactions", transactions, 12);
  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== spi_stream tests ===\n");
  mu_run_test(test_no_errors);
  return 0;
}
