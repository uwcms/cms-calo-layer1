################################################################################
####  Common Makefile fragments ################################################
################################################################################

# Compiler & linker options - from SDK GUI generated Makefile.
COMPILE_OPT=-DLITTLE_ENDIAN -Wall \
	    -I$(BSP)/microblaze_0/include \
	    -fmessage-length=0 -std=c99 -Wl,--no-relax \
	    -mlittle-endian -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LINK_OPT=-Wl,--no-relax -Wl,-T -Wl,src/lscript.ld \
	 -L$(BSP)/microblaze_0/lib -mlittle-endian \
	 -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LIBS=-Wl,--start-group,-lxil,-lgcc,-lc,--end-group

OPT=-O2
DEBUG=-g3
COMPILE=mb-gcc $(COMPILE_OPT) $(INCLUDES) $(OPT) $(DEBUG) $(LINK_OPT) $(LIBS)

#payload.elf: $(SRCS)
	#$(COMPILE) -o $@ $^

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
	rm -f *.elf
	rm -f *.elf.size
	rm -f *.elf.check
