cms-calo-layer1
===============

Xilinx Microblaze Projects for RCT Upgrade 

Installation
------------

Compilation requires installation of both the Xilinx EDK (for mb-gcc and libgen)
and a Petalinux installation (for compiling the soft-ipbus server).  See the
next section for initializing the environment.

To check out and build the trigger packages, do the following.

```shell
# 904 certificates are not installed correctly :(
export GIT_SSL_NO_VERIFY=1
cd $HOME/trigger_code/
# check out the softipbus dependency
svn co svn+ssh://svn.cern.ch/reps/cactus/trunk/cactuscore/softipbus $HOME/trigger_code/softipbus
# check out the ctp6commander client program
git clone https://github.com/uwcms/ctp6commander
# check out the set of packages
git clone https://github.com/uwcms/cms-calo-layer1.git
cd cms-calo-layer1
# Setup your environment for Xilinx build tools
source environment.sh
# compile the Board Support Packages (BSPs)
make cleanbsps
make bsps
# Make all the projects
make all
# Or just make one of them
cd orsc_fe_spi_echo_test 
make payload
```

More information about which HW and BSP files need to be tracked is available at the [Xilinx git info page.](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_4/SDK_Doc/reference/sdk_u_cvs.htm)


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

### orsc_fe_ipbus ###

Receives IPBus packets over SPI, reads and writes local memories, and returns
IPBus packets back over the serial link.

### orsc_be_ipbus ###

(TODO) Receives IPBus packets over VMEStream and forwards them along the SPI to
the oRSC front end.

### cp6_fe_uart_echo_test ###

Echoes everything it receives back along the UART. If you run this on the FE,
then executing the following on the BE
```
# Set TTY into RAW mode
stty -F /dev/ttyUL1 raw -echo
# Send data from BE->FE
echo "Go badgers" > /dev/ttyUL1
# Print echoed data
cat /dev/ttyUL1
```
should return the original string "Go badgers"


####################################
####  O R S C
####################################
### orsc_fe_sw ###

SW runs on Front End a.k.a Kintex in the oRSC board. Receives packets from the 
Back End a.k.a Spartan via UART. 
Packets are designed such a way to either fill the RAM on Kintex or 
Capture data from the JCC cables connected to the FE.

```shell
# Run using JTAG

fpga -f bitfiles/orsc/top_be.bit -debugdevice deviceNr 1
connect mb mdm -debugdevice deviceNr 1
rst
stop
dow orsc_be_sw/payload.elf
run

```

### orsc_be_sw ###

SW runs on the Back End. It waits to receive packets from the VME and
then forward it to the Front End. 
It also receives packets back from Front End and write it to RAM so that
VME can read it back...

```shell
# Run using JTAG

fpga -f bitfiles/orsc/top_fe.bit -debugdevice deviceNr 2
connect mb mdm -debugdevice deviceNr 2
rst
stop
dow orsc_fe_sw/payload.elf
run

```

Later,JTAG won't be used to load firmware into oRSC
The payload files will be merged into the firmware image file
and the card will activate the code and run on boot...

#### 


Notes from August Integration
-----------------------------

https://twiki.cern.ch/twiki/bin/view/CMS/ORSCCTP6Integration

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
