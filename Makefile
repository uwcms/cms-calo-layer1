SOFTDIR=/tmp/dbelknap/softipbus

CFLAGS:=-g -Wall -Iinclude -I$(SOFTDIR)/include -std=c99
CC=gcc

SRC:=$(wildcard src/vmestream/*.c) \
	$(SOFTDIR)/src/circular_buffer.c \
	$(SOFTDIR)/src/buffer.c

OBJ:=$(patsubst %.c,%.o,$(SRC))

LIB=lib/libvmestream.a

EXEC_SRC:=$(wildcard src/*.c)
EXEC:=$(patsubst src/%.c,bin/%,$(EXEC_SRC))

TEST_SRC:=$(wildcard tests/*_tests.c)
TESTS:=$(patsubst %.c,%,$(TEST_SRC))
SH_TESTS:=$(wildcard tests/*_tests.sh)

all : $(LIB) $(EXEC) tests

bin/% : $(LIB)
bin/% : src/%.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

$(LIB) : CFLAGS += -fPIC
$(LIB) : $(OBJ)
	@mkdir -p lib
	ar rcs $@ $(OBJ)
	ranlib $@

.PHONY : clean tests

tests : $(LIB)
tests : $(SH_TESTS)
tests : $(TESTS)
	@sh ./tests/runtests.sh

tests/%_tests : tests/%_tests.c
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

clean :
	rm -rf lib bin $(OBJ) $(TESTS)
	rm -f tests/tests.log
