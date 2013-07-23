
# Compiler & linker options - from SDK GUI generated Makefile.
# Note we had to change relativity of the paths.
COMPILE_OPT=-DLITTLE_ENDIAN -Wall -O0 -g3 \
	    -I../ctp6_fe_bsp/microblaze_0/include \
	    -I../common/buffer-types \
	    -I../common/spi-stream \
	    -fmessage-length=0 -std=c99 -Wl,--no-relax \
	    -mlittle-endian -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LINK_OPT=-Wl,--no-relax -Wl,-T -Wl,src/lscript.ld \
	 -L../ctp6_fe_bsp/microblaze_0/lib -mlittle-endian \
	 -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LIBS=-Wl,--start-group,-lxil,-lgcc,-lc,--end-group

OPT=-O0
DEBUG=-g3

# Make a single megalibrary of all user code.
SRCS=$(wildcard src/*.c) $(wildcard ../common/spi-stream/*.c) \
     $(wildcard ../common/buffer-types/*.c) 

program.elf: $(SRCS)
	mb-gcc $(COMPILE_OPT) $(OPT) $(DEBUG) $(LINK_OPT) $(LIBS) -o $@ $^
