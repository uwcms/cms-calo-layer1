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

# Initialize the eclipse workspace for the GUI.
# Following instructions from
# http://www.cs.indiana.edu/hmg/le/project-home/xilinx/ise_13.2/ISE_DS/EDK/eclipse/lin64/eclipse/plugins/com.xilinx.sdk.docs.user_1.0.0/reference/sdk_u_commandline.htm
XILINX_EDK=/opt/Xilinx/14.4/ISE_DS/EDK/
XILINX=/opt/Xilinx/14.4/ISE_DS/ISE
ECLIPSE=$(XILINX_EDK)/eclipse/lin/eclipse/eclipse
VM=$(XILINX)/java6/lin/jre/bin
WSPACE=$(PWD)
workspace:
	$(ECLIPSE) -vm $(VM) -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
	  -data $(WSPACE) \
	  -import ctp6_fe_hw -import ctp6_fe_bsp \
	  -import ctp6_be_hw \
	  -import orsc_fe_hw -import orsc_fe_bsp \
	  -import orsc_be_hw -import orsc_be_bsp \
	  -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole

.PHONY: workspace
