# Unit tests for sub-functionality used in spi_stream

# Compiles into native binaries.


# build guts
DEPOPTS = -MMD -MF .dep/`basename $@`.dep -MP
CCOPTS = $(DEPOPTS) -std=gnu99 -I"$(PWD)/.." -Wall -Werror 
SHAREDOPTS = -shared -fPIC

# This is an ugly hack to make sure we put the output into a directory
# where we can chmod +x it. (i.e. not on a windows VM disk)
OUT_DIR=/tmp/spi_test

test_%.o: %.c ../%.c
	gcc $(DEPOPTS) $(CCOPTS) $(SHAREDOPTS) -o $@ $< ../$*.c

test_%.exe: test_%.o minunit.c
	gcc $(CCOPTS) -o $@ minunit.c $< 
	mkdir -p $(OUT_DIR)
	cp $@ $(OUT_DIR)
	chmod 755 $(OUT_DIR)/$@

all: \
  test_circular_buffer.exe test_circular_buffer.o 
  #test_helloworld.exe test_helloworld.o

test: all
	LD_LIBRARY_PATH="$(PWD)" $(OUT_DIR)/test_circular_buffer.exe

distclean: clean
	rm -f .dep/*.dep 
clean:
	rm -f *.exe *.o

.PHONY: distclean clean all 
