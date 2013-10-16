#!/bin/bash

. /opt/Xilinx/14.4/ISE_DS/settings64.sh

# For reference - to setup Petalinux gcc
pushd /afs/hep.wisc.edu/home/uwhepfpga/petalinux-v12.12-final-full
source settings.sh
popd

# Setup an env var pointing to your softipbus installation
export SOFTIPBUS=/afs/hep.wisc.edu/cms/nwoods/trigger_code/softipbus
