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
#include <inttypes.h>

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
    int n_transactions, 
    u32 start_pkt_id) {

  // clobberable copy of the test data we send
  CircularBuffer* master_tx = cbuffer_copy(master_data);
  CircularBuffer* slave_tx = cbuffer_copy(slave_data);

  //printf("start: %li %li\n", cbuffer_size(master_tx), cbuffer_size(slave_tx));

  // where the "received" data goes
  CircularBuffer* master_rx = cbuffer_new();
  CircularBuffer* slave_rx = cbuffer_new();

  SPIStream* master_spi = spi_stream_init(
      master_tx, master_rx, master_transmit_callback, start_pkt_id);
  SPIStream* slave_spi = spi_stream_init(
      slave_tx, slave_rx, slave_transmit_callback, start_pkt_id);

  // if desired, advance the initial packet ID so we can test wrapping around.

  //printf("init:  %li %li\n", cbuffer_size(master_tx), cbuffer_size(slave_tx));


  for (int i = 0; i < n_transactions; ++i) {

    if (0 && i < 30) {
      printf("\niter %i: %"PRIx32" %"PRIx32"\n", i, cbuffer_size(master_tx), cbuffer_size(slave_tx));
    } 

    // check if we want to simulate mangling this data
    int master_error = 0;
    for (int j = 0; j < n_master_errors; ++j) {
      if (master_errors[j] == i) {
        //printf("master error @ %i\n", i);
        master_error = 1;
      }
    }
    int slave_error = 0;
    for (int j = 0; j < n_slave_errors; ++j) {
      if (slave_errors[j] == i) {
        //printf("slave error @ %i\n", i);
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

static char* test_delta_packet_id(void) {
  mu_assert_eq("normal", delta_packet_id(5, 4), 1);
  mu_assert_eq("inverted", delta_packet_id(4, 5), -1);
  mu_assert_eq("Size", 0xFFFFFFFF, (u32)(-1));
  mu_assert_eq("gt wraps", delta_packet_id(0, (u32)(-1)), 1);
  mu_assert_eq("lt wraps", delta_packet_id((u32)(-1), 0), -1);
  return 0;
}

static char* test_no_errors(void) {
  CircularBuffer* master_data = make_some_data(1000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // we should need 5000/(512 - 3) = 9.823182711 -> 10 interactions
  // we get one extra at the end, plus the initial 1 = 12
  return simulate_interaction(master_data, slave_data, 
      NULL, 0, NULL, 0, 12, 0);
}

static char* test_single_frame(void) {
  CircularBuffer* master_data = make_some_data(200, 2);
  CircularBuffer* slave_data = make_some_data(300, 3);
  // we need 1, get one extra at the end, plus the initial 1 = 3
  return simulate_interaction(master_data, slave_data, 
      NULL, 0, NULL, 0, 3, 0);
}

static char* test_single_error(void) {
  CircularBuffer* master_data = make_some_data(1000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on the master in 5th 
  u16 master_errors[1] = {5};
  // we should need 5000/(512 - 3) = 9.823182711 -> 10 interactions
  // we get one extra at the end, plus the initial 1 = 12
  // extra to recover from the error = 13
  return simulate_interaction(master_data, slave_data, 
      master_errors, 1, NULL, 0, 13, 0);
}

static char* test_multi_error(void) {
  CircularBuffer* master_data = make_some_data(1000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on the master in 5th 
  u16 master_errors[2] = {5, 7};
  // we should need 5000/(512 - 3) = 9.823182711 -> 10 interactions
  // we get one extra at the end, plus the initial 1 = 12
  // extra 3 to recover from each error = 14
  return simulate_interaction(master_data, slave_data, 
      master_errors, 2, NULL, 0, 15, 0);
}

static char* test_sequential_error(void) {
  CircularBuffer* master_data = make_some_data(1000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on the master in 5th 
  u16 master_errors[2] = {5, 6};
  // we should need 5000/(512 - 3) = 9.823182711 -> 10 interactions
  // we get one extra at the end, plus the initial 1 = 12
  // extra 6 to recover from each error = 14
  return simulate_interaction(master_data, slave_data, 
      master_errors, 2, NULL, 0, 13, 0);
}

static char* test_simultaneous_error(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on both in 5th 
  u16 master_errors[1] = {5};
  u16 slave_errors[1] = {5};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 1, slave_errors, 1, 13, 0);
}

static char* test_simultaneous_multiple_error(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on both in 5th 
  u16 master_errors[1] = {5};
  u16 slave_errors[3] = {5, 6, 7};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 1, slave_errors, 3, 18, 0);
}

static char* test_simultaneous_staggered_multiple_error(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on both in 5th 
  u16 master_errors[2] = {4, 5};
  u16 slave_errors[3] = {5, 6, 7};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 2, slave_errors, 3, 18, 0);
}

static char* test_simultaneous_a_lot_of_errors(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  // simulate error on both in 5th 
  u16 master_errors[2] = {4, 5};
  u16 slave_errors[8] = {5, 6, 7, 8, 9, 10, 11, 12};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 2, slave_errors, 8, 25, 0);
}

static char* test_wrap_around(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  return simulate_interaction(master_data, slave_data, 
      NULL, 0, NULL, 0, 25, 
      ((u64)1 << 32) - 4
      );
}

static char* test_wrap_around_errors(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  u16 master_errors[4] = {2, 3, 4, 5};
  u16 slave_errors[3] = {5, 6, 7};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 4, slave_errors, 3, 25, 
      ((u64)1 << 32) - 4
      );
}

static char* test_wrap_around_single_error(void) {
  CircularBuffer* master_data = make_some_data(5000, 2);
  CircularBuffer* slave_data = make_some_data(5000, 3);
  u16 master_errors[1] = {3};
  return simulate_interaction(master_data, slave_data, 
      master_errors, 1, NULL, 0, 20, 
      ((u64)1 << 32) - 4
      );
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== spi_stream tests ===\n");
  mu_run_test(test_delta_packet_id);
  mu_run_test(test_no_errors);
  mu_run_test(test_single_frame);
  mu_run_test(test_single_error);
  mu_run_test(test_multi_error);
  mu_run_test(test_sequential_error);
  mu_run_test(test_simultaneous_error);
  mu_run_test(test_simultaneous_multiple_error);
  mu_run_test(test_simultaneous_staggered_multiple_error);
  mu_run_test(test_simultaneous_a_lot_of_errors);
  mu_run_test(test_wrap_around);
  mu_run_test(test_wrap_around_errors);
  mu_run_test(test_wrap_around_single_error);
  return 0;
}
