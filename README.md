spi-stream
==========

A library which abstracts a bi-directional byte stream over a SPI interface.

The Xilinx SPI driver exchanges fixed size buffers between the master and slave
devices on each transfer, which is always initiated by the master.

Example
-------

```c
#include "circular_buffer.h"
#include "spi_stream.h"

// The device-driver dependent "transer complete" callback.
void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount) {
  u32 error = StatusEvent != XST_SPI_TRANSFER_DONE ? StatusEvent : 0;
  // Move SPI data in/out of the local IO buffers
  spi_stream_transfer_data(spi_stream, error);
}

// a call back which tells the device driver to swap the data.
void DoSpiTransfer(u8* tx, u8* rx, u16 nbytes) {
  XSpi_Transfer(&SpiInstance, tx, rx, nbytes);
}

// Program input/output buffers
CircularBuffer* tx_buffer = cbuffer_new();
CircularBuffer* rx_buffer = cbuffer_new();

SPIStream* spi_stream = spi_stream_init(
    tx_buffer, rx_buffer, 
    DoSpiTransfer, 
    0);

// Once the interrupts are setup correctly, data should transparently disappear
// from tx_buffer and appear in rx_buffer.  
```


Dependencies
------------

This package depends on the <buffer-types> library available at 
https://github.com/uwcms/buffer-types.  It should be installed in same directory
where the <spi-stream> library lives.

Unit Tests
----------

A comprehensive set of tests can be run:

```shell
cd tests
make test
```
