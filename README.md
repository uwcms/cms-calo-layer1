spi-stream
==========

A library which abstracts a bi-directional byte stream over a SPI interface.

The Xilinx SPI driver exchanges fixed size buffers between the master and slave
devices on each transfer, which is always initiated by the master.

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
