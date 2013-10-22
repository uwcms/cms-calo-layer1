################################################################################
####  Common Makefile fragments ################################################
################################################################################

ifndef LOG_LEVEL
    LOG_LEVEL=2
endif

# Compiler & linker options - from SDK GUI generated Makefile.
COMPILE_OPT=-DLITTLE_ENDIAN -Wall \
	    -I$(BSP)/microblaze_0/include \
	    -fmessage-length=0 -std=c99 -Wl,--no-relax \
	    -mlittle-endian -mxl-pattern-compare -mcpu=v8.40.b \
	    -mno-xl-soft-mul -DLOG_LEVEL=$(LOG_LEVEL)

LINK_OPT=-Wl,--no-relax -Wl,-T -Wl,src/lscript.ld \
	 -L$(BSP)/microblaze_0/lib -mlittle-endian \
	 -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul

LIBS=-Wl,--start-group,-lxil,-lgcc,-lc,--end-group

OPT=-O2
DEBUG=-g3
COMPILE=mb-gcc $(COMPILE_OPT) $(INCLUDES) $(OPT) $(DEBUG) $(LINK_OPT) $(LIBS)

ifndef LAYER1_DIR
	LAYER1_DIR=$(CURDIR)
endif

ifndef SOFTIPBUS
$(error SOFTIPBUS env variable is not set. It should probably be $(HOME)/trigger_code/softipbus)

endif

PROJECTS=\
	 $(LAYER1_DIR)/ctp6_fe_uart_ipbus \
	 $(LAYER1_DIR)/ctp6_fe_uart_echo_test \
	 $(LAYER1_DIR)/orsc_be_spi_echo_test \
	 $(LAYER1_DIR)/orsc_fe_spi_echo_test
	 #$LAYER1_DIR)/ctp6_fe_spi_echo_test \
	 #$LAYER1_DIR)/ctp6_fe_uart_blaster \
	 $(LAYER1_DIR)/orsc_fe_ipbus

all: bsps force_look
	@echo $(PWD)
	-for d in $(PROJECTS); do (cd $$d; $(MAKE) payload ); done

payload: payload.elf payload.elf.size payload.elf.check payload.S

payload.elf: $(SRCS)
	$(COMPILE) -o $@ $^

%.S: %.elf
	mb-objdump -S $< > $@

%.elf.size: %.elf
	mb-size $< | tee $@

%.elf.check: %.elf
	elfcheck -hw $(HW)/system.xml -pe microblaze_0 $< | tee $@

# Upload to the device
upload: payload.elf payload.elf.check
	echo "connect mb mdm -debugdevice deviceNr $(DEVICENR); rst; connect mb mdm -debugdevice deviceNr $(DEVICENR); dow payload.elf; run" | xmd

# Flash the ORSC bitfiles
orscbitfiles: orscbebits orscfebits

orscbebits:
	$(info Programming back end bitfile)
	echo "fpga -f bitfiles/orsc/top_be.bit -debugdevice deviceNr 1" | xmd

orscfebits:
	$(info Programming front end bitfile)
	echo "fpga -f bitfiles/orsc/top_fe.bit -debugdevice deviceNr 2" | xmd

# Compile the BSPs
bsps:
	cd $(LAYER1_DIR)/orsc_fe_bsp && make
	cd $(LAYER1_DIR)/orsc_be_bsp && make
	cd $(LAYER1_DIR)/ctp6_fe_bsp && make

clean:
	-for d in $(PROJECTS); do (cd $$d; rm -f *.elf* ); done

localclean: 
	rm -f payload.*

# http://owen.sj.ca.us/~rk/howto/slides/make/slides/makerecurs.html
# Force all subdirectories to be rechecked.
force_look:
	true

.PHONY: all bsps clean localclean payload upload orscbebits orscfebits orscbitfiles
