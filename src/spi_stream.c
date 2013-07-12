#include "spi_stream.h"

#include "circular_buffer.h"
#include "spi_stream_protocol.h"

// Size of the buffer we pass back and forth to the SPI device
#define SPI_BUFFER_SIZE 512 // words

static u32 spi_rx_buffer[SPI_BUFFER_SIZE];
static u32 spi_tx_buffer[SPI_BUFFER_SIZE];

static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;

void spi_stream_init(CircularBuffer* tx, CircularBuffer* rx) {
  tx_buffer = tx;
  rx_buffer = rx;
}

int spi_stream_transfer_data(int error) {
  // As long as the SPI device didn't screw up on receive, try to read the data
  // into the input buffer.   If we did have a local over run, some data was
  // lost, and we can't trust what is in the SPI buffer.
  if (!(error & SPI_STREAM_ERR_LOCAL_OVERRUN)) {
    error |= unescape_stream(rx_buffer, spi_rx_buffer, SPI_BUFFER_SIZE);
  }
  // If the remote side does not have any problem, send it new data, along 
  // with any errors we are experiencing locally.  If it does have a problem,
  // do nothing so we resend the last frame from what's already in spi_tx_buffer
  if (!(error & SPI_STREAM_ERR_REMOTE_MASK)) {
    escape_stream(spi_tx_buffer, SPI_BUFFER_SIZE, tx_buffer, 
        error & SPI_STREAM_ERR_LOCAL_MASK);
  }
  return error;
}

/*  
 *
Notes on Xilinx SPI codes and what they mean.

XST_SPI_MODE_FAULT	A mode fault error occurred, meaning another
				master tried to select this device as a slave
				when this device was configured to be a master.
				Any transfer in progress is aborted.



XST_SPI_TRANSFER_DONE	The requested data transfer is done



XST_SPI_TRANSMIT_UNDERRUN	As a slave device, the master clocked
				data but there were none available in the
				transmit register/FIFO. This typically means the
				slave application did not issue a transfer
				request fast enough, or the processor/driver
				could not fill the transmit register/FIFO fast
				enough.



XST_SPI_RECEIVE_OVERRUN	The SPI device lost data. Data was received
				but the receive data register/FIFO was full.
				This indicates that the device is receiving data
				faster than the processor/driver can consume it.



XST_SPI_SLAVE_MODE_FAULT	A slave SPI device was selected as a slave while
				it was disabled.  This indicates the master is
				already transferring data (which is being
				dropped until the slave application issues a
				transfer).
 *
 *  */
