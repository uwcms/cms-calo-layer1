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
	echo "connect mb mdm -debugDevice deviceNr 2; mwr 0x100F0034 0x1" | xmd

readout:
	$(info Triggering capture)
	echo "connect mb mdm -debugDevice deviceNr 2; mrd 0x10084000 1024" | xmd | grep -v 00000000

inject: 
	cd ~rctpro/904Tests/ && ./gct_output_all.sh

