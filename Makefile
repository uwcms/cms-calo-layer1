SOFTDIR=/Users/austin/Documents/CMS/softipbus

CFLAGS=-Wall -I./include/ -I$(SOFTDIR)/include/
CC=gcc

bin/vme2fd : src/vme2fd.c
	mkdir -p ./bin
	gcc $(CFLAGS) -o $@ $<

test : bin/vme2fd
	./scripts/vme2fd_test.sh

tests : tests/bin/VMEStream_tests
	./tests/bin/VMEStream_tests

VME_OBJECTS=tests/VMEStream_tests.o src/VMEStream_PC.o $(SOFTDIR)/src/circular_buffer.o $(SOFTDIR)/src/buffer.o

tests/bin/VMEStream_tests : $(VME_OBJECTS)
	mkdir -p ./tests/bin
	$(CC) $^ -o $@
