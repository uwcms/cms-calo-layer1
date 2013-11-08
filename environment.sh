#!/bin/bash

. /opt/Xilinx/14.4/ISE_DS/settings64.sh

# For reference - to setup Petalinux gcc
if [ -d /opt/Xilinx/petalinux-v12.12-final-full/ ]; then
    PETALINUXDIR=/opt/Xilinx/petalinux-v12.12-final-full/
else
    PETALINUXDIR=/afs/hep.wisc.edu/home/uwhepfpga/petalinux-v12.12-final-full 
fi

pushd $PETALINUXDIR
source settings.sh
popd


# Setup an env var pointing to your softipbus installation
export SOFTIPBUS=${PWD}/softipbus
export BITBANGER=${PWD}/bitbanger2

# Include xdaq libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/xdaq/lib

# Run VMEStream in ECHOTEST mode (for now)
export VME_CONTROLLER=CAEN

export RCTSTUFF=/cms/cmslab/trigger_upgrade/cactusprojects/rct

export PATH=$RCTSTUFF/bin:$PATH
export LD_LIBRARY_PATH=$RCTSTUFF/lib:$LD_LIBRARY_PATH
