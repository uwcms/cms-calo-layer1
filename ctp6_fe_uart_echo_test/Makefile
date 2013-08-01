# CTP6 front uart test

HW=../ctp6_fe_hw
BSP=../ctp6_fe_bsp

INCLUDES=-I../common/buffer-types -I../common/soft-ipbus/inc

include ../Makefile
OPT=-O0
DEBUG=-g0

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/buffer-types/*.c) 

ctp6_fe_uart_echo_test.elf: $(SRCS)
	$(COMPILE) -DLOG_LEVEL=2 -o $@ $^

all: ctp6_fe_uart_echo_test.elf \
  ctp6_fe_uart_echo_test.elf.size \
  ctp6_fe_uart_echo_test.elf.check

upload: all
	echo "connect mb mdm; dow ctp6_fe_uart_echo_test.elf;" | xmd
