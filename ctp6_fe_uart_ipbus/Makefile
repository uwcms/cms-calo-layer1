# CTP6 front-end IPBus server (over UART) 

BSP=../ctp6_fe_bsp
HW=../ctp6_fe_hw

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include

OPT=-O0

LAYER1_DIR=..

# In JTAG chain, needed for make upload
DEVICENR=2

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) \
     $(wildcard $(IPBUS)/src/*buffer*c) \
     $(IPBUS)/src/serialization.c $(IPBUS)/src/packethandler.c \
     $(IPBUS)/src/transactionhandler.c \
     $(IPBUS)/src/handlers.c \
     $(IPBUS)/src/nullmembase.c

include ../Makefile
