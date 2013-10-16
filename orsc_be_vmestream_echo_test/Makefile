# oRSC backend VMEStream test

HW=../orsc_be_hw
BSP=../orsc_be_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include -I../VMEStream/include

LAYER1_DIR=..

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../VMEStream/src/vmestream/VMEStream_PC.c) \
     $(wildcard $(SOFTIPBUS)/src/*buffer*.c)

include ../Makefile

