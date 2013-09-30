# oRSC front SPI test

HW=../orsc_fe_hw
BSP=../orsc_fe_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include -I../spistream

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../spistream/*.c) \
     $(wildcard $(SOFTIPBUS)/src/*buffer*.c)

include ../Makefile
