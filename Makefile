################################################################################
####  Common Makefile fragments ################################################
################################################################################

# Compiler & linker options - from SDK GUI generated Makefile.
COMPILE_OPT=-DLITTLE_ENDIAN -Wall \
	    -I$(BSP)/microblaze_0/include \
	    -fmessage-length=0 -std=c99 -Wl,--no-relax \
	    -mlittle-endian -mxl-pattern-compare -mcpu=v8.40.b \
	    -mno-xl-soft-mul -DLOG_LEVEL=2 

LINK_OPT=-Wl,--no-relax -Wl,-T -Wl,src/lscript.ld \
	 -L$(BSP)/microblaze_0/lib -mlittle-endian \
	 -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LIBS=-Wl,--start-group,-lxil,-lgcc,-lc,--end-group

OPT=-O2
DEBUG=-g3
COMPILE=mb-gcc $(COMPILE_OPT) $(INCLUDES) $(OPT) $(DEBUG) $(LINK_OPT) $(LIBS)

PROJECTS=\
	 ctp6_fe_uart_ipbus
	 #ctp6_fe_spi_echo_test \
	 #ctp6_fe_uart_blaster \
	 #ctp6_fe_uart_echo_test \
	 #orsc_be_spi_echo_test \
	 #orsc_fe_spi_echo_test \
	 orsc_fe_ipbus 

all: bsps force_look
	-for d in $(PROJECTS); do (cd $$d; $(MAKE) payload ); done

payload: payload.elf payload.elf.size payload.elf.check

payload.elf: $(SRCS) 
	$(COMPILE) -o $@ $^

%.elf.size: %.elf
	mb-size $< | tee $@

%.elf.check: %.elf
	elfcheck -hw $(HW)/system.xml -pe microblaze_0 $< | tee $@

# Compile the BSPs
bsps:
	cd orsc_fe_bsp && make
	cd orsc_be_bsp && make
	cd ctp6_fe_bsp && make

clean: 
	-for d in $(PROJECTS); do (cd $$d; rm -f *.elf* ); done

# http://owen.sj.ca.us/~rk/howto/slides/make/slides/makerecurs.html
# Force all subdirectories to be rechecked.
force_look:
	true

.PHONY: all bsps clean payload
