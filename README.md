rct-upgrade-microblaze
======================

Xilinx Microblaze Projects for RCT Upgrade 

Packages
--------

In general, for each board there is a HW and BSP package.  These are generated
using the Xilinx SDK GUI using HW .xml file provided by Mathias.  These contain
information about the devices attached to the processor.  The BSP contains all
the driver headers and libraries for a given device.   There are a number of
standalone programs for each of the boards.   Most of them are small tests of
various device functionality (UART or SPI, etc).  The production ones are:

### cp6_fe_uart_ipbus ###

Receives IPBus packets over UART, reads and writes local memories, and returns
IPBus packets back over the serial link.

### orsc_fe_spi_ipbus ###

Receives IPBus packets over SPI, reads and writes local memories, and returns
IPBus packets back over the serial link.

### cp6_fe_uart_echo_test ###

Echoes everything it receives back along the UART. If you run this on the FE,
then executing the following on the BE
```
# Set TTY into RAW mode
stty -F /dev/ttyUL1 raw
# Send data from BE->FE
echo "Go badgers" > /dev/ttyUL1
# Print echoed data
cat /dev/ttyUL1
```
should return the original string "Go badgers"


### orsc_(fe|be)_spi_echo_test ###

Untested. Should send test data back and forth between the oRSC BE and FE via
the inter-FPGA SPI.


Installation
------------

Compilation requires installation of both the Xilinx EDK (for mb-gcc and libgen)
and a Petalinux installation (for compiling the soft-ipbus server).  See the
next section for initializing the environment.

To check out and build the trigger packages, do the following.

```shell
cd $HOME/trigger_code/
# check out the softipbus dependency
svn co svn+ssh://svn.cern.ch/reps/cactus/trunk/cactuscore/softipbus $HOME/trigger_code/softipbus
# check out the set of packages
git clone https://github.com/uwcms/cms-calo-layer1.git
cd cms-calo-layer1
# Setup your environment for Xilinx build tools
source environment.sh
# compile the Board Support Packages (BSPs)
make bsps
# Make all the projects
make all
# Or just make one of them
cd orsc_fe_spi_echo_test 
make payload
```

More information about which HW and BSP files need to be tracked is available at the [Xilinx git info page.](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_4/SDK_Doc/reference/sdk_u_cvs.htm)


Miscellany
----------

Info from Jes on mapping a petalinux ``/dev/ttyX`` UART to the correct device on
the standalone FPGA.

> They are accessed through /dev/ttyUL# for some #.
> 
> There are four UART in linux.  The two you are interested in are mapped to
> memory addresses 50000000 and 70000000 (for link 0 and 1 respectively).
> 
> To determine the /dev/ttyUL# for a given instance, run the following on
> the linux system:
> 
> ``$ ls /sys/bus/platform/drivers/uartlite/50000000.serial/tty``
> 
> Replace 50000000 with the relevant address.
> 
> This will return something like "ttyUL1", therefore use /dev/ttyUL1 to
> access this device.
> 
> This /dev/ttyUL1 will remain constant unless the backend FPGA image is
> altered in certain ways.  The above method will remain reliable unless the
> exact address of these specific devices changes.
