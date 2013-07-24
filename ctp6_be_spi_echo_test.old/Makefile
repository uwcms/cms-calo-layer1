# CTP6 backend SPI test

HW=../ctp6_be_hw
BSP=../ctp6_be_bsp

INCLUDES=-I../common/buffer-types \
	 -I../common/spi-stream


include ../Makefile

COMPILE_OPT=-Wall \
	    -I$(BSP)/microblaze_0/include \
	    -ggdb -std=gnu99

LINK_OPT=-Wl,--no-relax -Wl,-T -Wl,src/lscript.ld \
	 -L$(BSP)/microblaze_0/lib 

COMPILE=mb-gcc $(COMPILE_OPT) $(INCLUDES) $(OPT) $(DEBUG) $(LINK_OPT) 

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/spi-stream/*.c) \
     $(wildcard ../common/buffer-types/*.c) 

ctp6_be_spi_echo_test.elf: $(SRCS)
	$(COMPILE) -o $@ $^

all: ctp6_be_spi_echo_test.elf 

upload: all
	scp ctp6_be_spi_echo_test.elf root@192.168.1.31:/tmp/
