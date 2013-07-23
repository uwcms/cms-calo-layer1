# Following instructions from
# http://www.cs.indiana.edu/hmg/le/project-home/xilinx/ise_13.2/ISE_DS/EDK/eclipse/lin64/eclipse/plugins/com.xilinx.sdk.docs.user_1.0.0/reference/sdk_u_commandline.htm
XILINX_EDK=/opt/Xilinx/14.4/ISE_DS/EDK/
XILINX=/opt/Xilinx/14.4/ISE_DS/ISE
ECLIPSE=$(XILINX_EDK)/eclipse/lin/eclipse/eclipse
VM=$(XILINX)/java6/lin/jre/bin

WSPACE=$(PWD)

# Initialize the eclipse workspace for the GUI.
workspace:
	$(ECLIPSE) -vm $(VM) -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
	  -data $(WSPACE) \
	  -import ctp6_fe_hw -import ctp6_fe_bsp \
	  -import ctp6_be_hw \
	  -import orsc_fe_hw -import orsc_fe_bsp \
	  -import orsc_be_hw -import orsc_be_bsp \
	  -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole

# Compile the BSPs
bsps:
	cd orsc_fe_bsp && make
	cd orsc_be_bsp && make
	cd ctp6_fe_bsp && make

.PHONY: workspace
