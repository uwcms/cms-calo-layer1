SOFTDIR=/afs/cern.ch/user/d/dbelknap/softipbus

CFLAGS:=-g -Wall -Iinclude -I$(SOFTDIR)/include -I/opt/xdaq/include/ 
CXXFLAGS:=$(CFLAGS) -DLINUX
LDFLAGS:=-L/opt/xdaq/lib/ -lCAENVME -llog4cplus
CC=gcc
CXX=g++

SRC:=$(wildcard src/vmestream/*.c) \
	$(wildcard src/vmestream/*.cc) \
	$(SOFTDIR)/src/circular_buffer.c \
	$(SOFTDIR)/src/buffer.c

OBJ:=$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(SRC)))

LIB=lib/libvmestream.a

EXEC_SRC:=$(wildcard src/*.cc)
EXEC:=$(patsubst src/%.cc,bin/%,$(EXEC_SRC))

TEST_SRC:=$(wildcard tests/*_tests.c)
TESTS:=$(patsubst %.c,%,$(TEST_SRC))
SH_TESTS:=$(wildcard tests/*_tests.sh)

all : $(LIB) $(EXEC) tests

bin/% : $(LIB)
bin/% : src/%.cc
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LIB)

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
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIB)

clean :
	rm -rf lib bin $(OBJ) $(TESTS)
	rm -f tests/tests.log
