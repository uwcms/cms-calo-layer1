# oRSC backend SPI test

HW=../orsc_be_hw
BSP=../orsc_be_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include -I../spistream

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../spistream/*.c) \
     $(wildcard $(SOFTIPBUS)/src/*buffer*.c)

include ../Makefile
