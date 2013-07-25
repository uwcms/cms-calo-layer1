# CTP6 backend SPI test

BSP=../ctp6_be_bsp
# Makefile for cross-compiling soft-ipbus to Petalinux 

PETALINUX=/scratch/efriis/petalinux-v12.12-final-full
PETADIST=$(PETALINUX)/software/petalinux-dist

CC=$(PETALINUX)/tools/linux-i386/microblazeel-xilinx-linux-gnu/bin/microblazeel-xilinx-linux-gnu-gcc

INCLUDES=-I$(PETADIST)/stage/include \
	 -I$(PETADIST)/stage/usr/include \
	 -I$(PETADIST)/stage/usr/local/include \
	 -I$(PETADIST) -I$(PETADIST)/include \
	 -I../common/buffer-types \
	 -I../common/spi-stream \
	 -I$(BSP)/microblaze_0/include \
	 -Iinc

CCOPTS=-fomit-frame-pointer -pipe -fno-common -fno-builtin \
       -mxl-multiply-high -mno-xl-soft-mul -mno-xl-soft-div \
       -mxl-barrel-shift -mxl-pattern-compare -mhard-float -mcpu=v8.40.b \
       -Wall -Werror -std=gnu99 \
       -DEMBED -Dlinux -D__linux__ -Dunix -fPIC -DLINUX \
       -DLOG_LEVEL=0x03


LINKOPTS=-L$(PETADIST)/lib -L$(PETADIST)/stage/lib \
	 -L$(PETADIST)/stage/usr/lib -L$(PETADIST)/stage/usr/local/lib \
	 -L$(BSP)/microblaze_0/lib \
	 -Wl,--no-relax -Wl,-T -Wl,src/lscript.ld

LIBS=-Wl,--start-group,-lxil,-lgcc,-lc,--end-group

COMPILE=$(CC) $(INCLUDES) $(CCOPTS) $(LINKOPTS) $(LIBS)

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/spi-stream/*.c) \
     $(wildcard ../common/buffer-types/*.c) 

ctp6_be_spi_echo_test: $(SRCS)
	$(COMPILE) -o $@ $^

all: ctp6_be_spi_echo_test

upload: all
	scp ctp6_be_spi_echo_test root@192.168.1.31:/tmp/
