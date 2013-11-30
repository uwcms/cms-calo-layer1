# oRSC front-end UART echo test.  Echoes everything received on the UART.

HW=../orsc_fe_hw
BSP=../orsc_fe_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include -I../tracer

LAYER1_DIR=..

# In JTAG chain, needed for make upload
DEVICENR=2

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard $(IPBUS)/src/*buffer*c) ../tracer/tracer.c

include ../Makefile

OPT=-O2
DEBUG=-g3
LOG_LEVEL=0x0
