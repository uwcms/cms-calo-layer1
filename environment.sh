#!/bin/bash

. /opt/Xilinx/14.4/ISE_DS/settings64.sh

# For reference - to setup Petalinux gcc
# pushd /afs/hep.wisc.edu/home/uwhepfpga/petalinux-v12.12-final-full
# source settings.sh
# popd

# Setup an env var pointing to your softipbus installation
export SOFTIPBUS=$HOME/trigger_code/softipbus

# Include xdaq libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/xdaq/lib

# Run VMEStream in ECHOTEST mode (for now)
export VME_CONTROLLER=TESTECHO

if [ -f /nfshome0/rctts/cactusprojects/rct/environment.sh ]; then
    source /nfshome0/rctts/cactusprojects/rct/environment.sh
fi

