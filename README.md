rct-upgrade-microblaze
======================

Xilinx Microblaze Projects for RCT Upgrade 

Installation
------------

There is some rudimentary (awful) support for putting all the eclipse plumbing
together using the command line. In general, there are normal Makefiles for each
project.  The correct compiler/linker flags are first cajoling eclipse to build
the project, then copying the options into the Makefiles.

```shell
# check out the set of packages
git clone --recursive https://github.com/uwcms/rct-upgrade-microblaze.git
cd rct-upgrade-microblaze
# initialize the Eclipse workspace
make workspace
# compile the Board Support Packages (BSPs)
make bsps
# Make one of the subprojects
cd orsc_fe_spi_echo_test 
make all
```

More information is available at the [Xilinx git info page.](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_4/SDK_Doc/reference/sdk_u_cvs.htm)
and the [Xilinx command line flows.](http://www.cs.indiana.edu/hmg/le/project-home/xilinx/ise_13.2/ISE_DS/EDK/eclipse/lin64/eclipse/plugins/com.xilinx.sdk.docs.user_1.0.0/reference/sdk_u_commandline.htm)

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
