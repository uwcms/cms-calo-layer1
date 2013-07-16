# Unit tests for sub-functionality used in spi_stream

# Compiles into native binaries.


# build guts
DEPOPTS = -MMD -MF .dep/`basename $@`.dep -MP
CCOPTS = $(DEPOPTS) -std=gnu99 -I"$(PWD)/.." -Wall -Werror  -DUSE_INTTYPES
SHAREDOPTS = -shared -fPIC

# This is an ugly hack to make sure we put the output into a directory
# where we can chmod +x it. (i.e. not on a windows VM disk)
OUT_DIR=/tmp/spi_test

SRCS=../circular_buffer.c ../spi_stream.c ../adler32.c ../protocol.c

test_%.o: %.c ../%.c
	gcc $(DEPOPTS) $(CCOPTS) $(SHAREDOPTS) -o $@ $< $(SRCS)

test_%.exe: test_%.o minunit.c
	gcc $(CCOPTS) -o $@ minunit.c $<  
	mkdir -p $(OUT_DIR)
	cp $@ $(OUT_DIR)
	chmod 755 $(OUT_DIR)/$@

all: \
  test_circular_buffer.exe test_circular_buffer.o \
  test_adler32.exe test_adler32.o \
  test_protocol.exe test_protocol.o \
  test_spi_stream.exe test_spi_stream.o 
  #test_helloworld.exe test_helloworld.o

test: all
	LD_LIBRARY_PATH="$(PWD)" $(OUT_DIR)/test_circular_buffer.exe
	LD_LIBRARY_PATH="$(PWD)" $(OUT_DIR)/test_adler32.exe
	LD_LIBRARY_PATH="$(PWD)" $(OUT_DIR)/test_protocol.exe
	LD_LIBRARY_PATH="$(PWD)" $(OUT_DIR)/test_spi_stream.exe

distclean: clean
	rm -f .dep/*.dep 
clean:
	rm -f *.exe *.o

.PHONY: distclean clean all 
