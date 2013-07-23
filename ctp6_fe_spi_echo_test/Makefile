# CTP6 front SPI test

HW=../ctp6_fe_hw
BSP=../ctp6_fe_bsp

INCLUDES=-I../common/buffer-types \
	 -I../common/spi-stream

include ../Makefile

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/spi-stream/*.c) \
     $(wildcard ../common/buffer-types/*.c) 

ctp6_fe_spi_echo_test.elf: $(SRCS)
	$(COMPILE) -o $@ $^

all: ctp6_fe_spi_echo_test.elf \
  ctp6_fe_spi_echo_test.elf.size \
  ctp6_fe_spi_echo_test.elf.check

