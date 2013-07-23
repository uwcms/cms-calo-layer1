rct-upgrade-microblaze
======================

Xilinx Microblaze Projects for RCT Upgrade 

Installation
------------

There is some rudimentary (awful) support for putting all the eclipse plumbing
together using the command line.  You may need to check the paths to the eclipse
binaries are correct in the Makefile.  You can compile the code like so:

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
make program.elf
```

More information is available at the [Xilinx git info page.](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_4/SDK_Doc/reference/sdk_u_cvs.htm)
and the [Xilinx command line flows.](http://www.cs.indiana.edu/hmg/le/project-home/xilinx/ise_13.2/ISE_DS/EDK/eclipse/lin64/eclipse/plugins/com.xilinx.sdk.docs.user_1.0.0/reference/sdk_u_commandline.htm)
