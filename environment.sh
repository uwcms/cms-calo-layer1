#!/bin/bash

. /opt/Xilinx/14.4/ISE_DS/settings64.sh

# For reference - to setup Petalinux gcc
if [ -d /afs/hep.wisc.edu/home/uwhepfpga/petalinux-v12.12-final-full ]; then
    PETALINUXDIR=/afs/hep.wisc.edu/home/uwhepfpga/petalinux-v12.12-final-full 
else
    PETALINUXDIR=/opt/Xilinx/petalinux-v12.12-final-full/
fi

pushd $PETALINUXDIR
source settings.sh
popd


# Setup an env var pointing to your softipbus installation
export SOFTIPBUS=$HOME/trigger_code/softipbus

# Include xdaq libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/xdaq/lib

# Run VMEStream in ECHOTEST mode (for now)
export VME_CONTROLLER=TESTECHO

export RCTSTUFF=/nfshome0/rctts/cactusprojects/rct/

export PATH=$RCTSTUFF/bin:$PATH
export LD_LIBRARY_PATH=$RCTSTUFF/lib:$LD_LIBRARY_PATH
