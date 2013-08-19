# CTP6 front uart test

HW=../ctp6_fe_hw
BSP=../ctp6_fe_bsp

IPBUS=$(SOFTIPBUS)

INCLUDES=-I$(IPBUS)/include

include ../Makefile

OPT=-O0

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) \
     $(wildcard $(IPBUS)/src/*buffer*c) \
     $(IPBUS)/src/serialization.c $(IPBUS)/src/packethandler.c \
     $(IPBUS)/src/serialization.c $(IPBUS)/src/packethandler.c \
     $(IPBUS)/src/transactionhandler.c \
     $(IPBUS)/src/handlers.c \
     $(IPBUS)/src/nullmembase.c

ctp6_fe_uart_ipbus.elf: $(SRCS)
	$(COMPILE) -DLOG_LEVEL=2 -o $@ $^

all: ctp6_fe_uart_ipbus.elf \
  ctp6_fe_uart_ipbus.elf.size \
  ctp6_fe_uart_ipbus.elf.check

upload: all
	echo "connect mb mdm; dow ctp6_fe_uart_ipbus.elf;" | xmd
