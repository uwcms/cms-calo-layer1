# Unit tests for sub-functionality used in spi_stream

# Compiles into native binaries.


# build guts
DEPOPTS = -MMD -MF .dep/`basename $@`.dep -MP
CCOPTS = $(DEPOPTS) -std=gnu99 -I$(PWD)/inc -Wall -Werror 
SHAREDOPTS = 

# This is an ugly hack to make sure we put the output into a directory
# where we can chmod +x it. (i.e. not on a windows VM disk)
OUT_DIR=/tmp/spi_test

$(OUT_DIR)/test_%.exe: %.c
	mkdir -p $(OUT_DIR)
	gcc $(CCOPTS) $(SHAREDOPTS) -o $@ $<
	chmod 755 $@

all: $(OUT_DIR)/test_helloworld.exe

test: all
	$(OUT_DIR)/test_helloworld.exe

distclean: clean
	rm -f .dep/*.dep 
clean:
	rm -f $(OUT_DIR)/*.exe

.PHONY: distclean clean all 
