# CTP6 front uart test

HW=../ctp6_fe_hw
BSP=../ctp6_fe_bsp

INCLUDES=-I../common/buffer-types

include ../Makefile
OPT=-O0

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/buffer-types/*.c) 
#SRCS=src/platform.c ../peripheral_examples/xuartlite_intr_example.c $(wildcard ../common/buffer-types/*.c) 

ctp6_fe_uart_echo_test.elf: $(SRCS)
	$(COMPILE) -o $@ $^

all: ctp6_fe_uart_echo_test.elf \
  ctp6_fe_uart_echo_test.elf.size \
  ctp6_fe_uart_echo_test.elf.check

upload: all
	echo "connect mb mdm; dow ctp6_fe_uart_echo_test.elf;" | xmd
