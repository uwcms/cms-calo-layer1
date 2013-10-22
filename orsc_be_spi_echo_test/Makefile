# oRSC backend SPI test

HW=../orsc_be_hw
BSP=../orsc_be_bsp

IPBUS=$(SOFTIPBUS)

# In JTAG chain, needed for make upload
DEVICENR=1

INCLUDES=-I$(IPBUS)/include -I../spistream

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../spistream/*.c) \
     $(wildcard $(SOFTIPBUS)/src/*buffer*.c)

include ../Makefile

# No logging
LOG_LEVEL=0
