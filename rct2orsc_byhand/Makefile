# Commands for running the rct2orsc pattern test
#
# Run this on l1ts-rct-02
#
# Targets:
#
# setup: flashes the oRSC FE bitfile and caenRCTBoots the crate.

LAYER1_DIR=..

include ../Makefile

setup: orscfebits dow

dow:
	$(info Loading a dummy payload)
	echo "connect mb mdm -debugDevice deviceNr 2; dow payload.elf; run" | xmd

trigger:
	$(info Triggering capture)
	echo "connect mb mdm -debugDevice deviceNr 2; rst; stop; mwr 0x100F0034 0x1" | xmd

readout:
	$(info Reading out capture RAMS)
	echo "connect mb mdm -debugDevice deviceNr 2; rst; stop; mrd 0x10084000 64" | xmd 

lightsout:
	$(info Turning off LEDs)
	echo "connect mb mdm -debugDevice deviceNr 2; rst; stop; \
	    mwr 0x10008000 0x0; \
	    mwr 0x10008004 0x0; \
	    mwr 0x10008008 0x0; \
	    mwr 0x1000800C 0x0; \
	    mwr 0x10008010 0x0; \
	    mwr 0x10008014 0x0; \
	    mwr 0x10008018 0x0; \
	    mwr 0x1000801C 0x0; \
	    mwr 0x10008020 0x0;" | xmd

lightson:
	$(info Turning on LEDs)
	echo "connect mb mdm -debugDevice deviceNr 2; rst; stop; \
	    mwr 0x10008000 0x1; \
	    mwr 0x10008004 0x1; \
	    mwr 0x10008008 0x1; \
	    mwr 0x1000800C 0x1; \
	    mwr 0x10008010 0x1; \
	    mwr 0x10008014 0x1; \
	    mwr 0x10008018 0x1; \
	    mwr 0x1000801C 0x1; \
	    mwr 0x10008020 0x1; \
	    mrd 0x10008000 0x1; \
	    mrd 0x10008004 0x1; \
	    mrd 0x10008008 0x1; \
	    mrd 0x1000800C 0x1; \
	    mrd 0x10008010 0x1; \
	    mrd 0x10008014 0x1; \
	    mrd 0x10008018 0x1; \
	    mrd 0x1000801C 0x1; \
	    mrd 0x10008020 0x1; " | xmd

inject: 
	cd ~rctpro/904Tests/ && ./gct_output_all.sh

