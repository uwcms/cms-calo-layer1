#include "spi_stream.h"

#include "circular_buffer.h"
#include "spi_stream_protocol.h"

static void swap_tx_buffers(SPIStream* stream) {
  u32* temp = stream->spi_tx_buffer_prev;
  stream->spi_tx_buffer_prev = stream->spi_tx_buffer_current;
  stream->spi_tx_buffer_current = temp;
}

void spi_stream_init(SPIStream* stream, CircularBuffer* tx, CircularBuffer* rx) {
  stream->tx_buffer = tx;
  stream->rx_buffer = rx;
  stream->waiting_for_packet_id = 0;
  stream->stream_state = SPI_STREAM_STATE_NORMAL;
}

int spi_stream_transfer_data(SPIStream* stream, int error) {
  // verify the incoming data.
  int cksum_error = 0;
  u32 received_packet_id = spi_stream_verify_data(
      stream->spi_rx_buffer, SPI_BUFFER_SIZE, &cksum_error);

  switch (stream->spi_stream_state) {


    case SPI_STREAM_STATE_NORMAL:
      if (cksum_error) {
        // continue to resend the current TX buffer with same packet ID
        // ignore the RX buffer.
        stream->spi_stream_state = SPI_STREAM_STATE_WAITING;
      } else {
        switch (received_packet_id) {
          case stream->waiting_for_packet_id:
            // Got what we wanted, read the data.
            int rx_buffer_ok = spi_stream_read_rx_packet(
                stream->spi_rx_buffer, stream->rx_buffer);
            //
            if (rx_buffer_ok) {
              // Now send next packet in the sequence
              stream->waiting_for_packet_id++;
              // keep track of previously sent buffer
              swap_tx_buffers(stream);
              // build the next TX buffer
              spi_stream_construct_tx_packet(
                  stream->waiting_for_packet_id, 
                  stream->spi_tx_buffer_current, 
                  SPI_BUFFER_SIZE,
                  stream->tx_buffer);
              break;
            } else {
              // we overflowed the RX buffer, move to WAIT mode.
              stream-spi_stream_state = SPI_STREAM_STATE_WAITING;
            }
          case stream->waiting_for_packet_id - 1:
            // Move to resend mode.  Send previous packet (i.e. do nothing).
            stream->spi_stream_state = SPI_STREAM_STATE_RESEND;
            break;
        }
      }
      break;

    case SPI_STREAM_STATE_WAITING:
      // we are waiting for the remote to resend a packet we missed.
      if (cksum_error || 
          received_packet_id == stream->waiting_for_packet_id + 1) {
        // keep sending until we get the one we want.
      } else if (!cksum_error && 
          received_packet_id == stream->waiting_for_packet_id) {
        // we got the one we want
      }

  }

}

/*  States
 

    Each time we exchange a packet, we see what packet ID the other side thought
    on the previous exchange.
 

    OK - everything is OK
        * local error = none
        * remote error = none
        Action: transmit 

    Local overrun - local SPI device buffer was overflowed, data was lost.
        * local error = SPI_STREAM_ERR_LOCAL_OVERRUN
        * remote error = unknown
        Action: 

    Local error - program buffer overflowed, 

 *  */

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
