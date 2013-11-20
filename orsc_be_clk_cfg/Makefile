# oRSC backend SPI test

HW=../orsc_be_hw
BSP=../orsc_be_bsp

IPBUS=$(SOFTIPBUS)
BB=$(BITBANGER)

# In JTAG chain, needed for make upload
DEVICENR=1

INCLUDES=-I$(IPBUS)/include -I../tracer -I../bitbanger2

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) \
     $(wildcard $(SOFTIPBUS)/src/*buffer*.c) \
     $(wildcard $(BITBANGER)/*utils.c) \
     ../tracer/tracer.c 

include ../Makefile

# No logging
LOG_LEVEL=0
DEBUG=-g3
OPT=-O2
