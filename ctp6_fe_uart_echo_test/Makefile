# CTP6 front-end UART echo test.  Echoes everything received on the UART.

HW=../ctp6_fe_hw
BSP=../ctp6_fe_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include

LAYER1_DIR=..

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard $(IPBUS)/src/*buffer*c)

include ../Makefile
