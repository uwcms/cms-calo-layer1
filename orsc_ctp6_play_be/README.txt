### ### ### ### ### ### ### ### ### 
#  oRSC BE and FE code explanation \
#
#  Read/Write data From/To Kintex \
#  Addresses (RAM and REG)
### ### ### ### ### ### ### ### ### 


# Program BE and FE FPGAs via VME pc 
# Eventually this next 3 lines will \
# be implemented as a default in \
# the Firmware image

# Set up below need to be done just ONCE \
# Unless there is a firmware/software change 

echo "fpga -f bitfiles/orsc/top_be.bit -debugdevice deviceNr 1 && fpga -f bitfiles/orsc/top_fe.bit -debugdevice deviceNr 2" | xmd
echo "connect mb mdm -debugdevice deviceNr 1; rst; stop; dow orsc_ctp6_play_be/payload.elf && run" | xmd
echo "connect mb mdm -debugdevice deviceNr 2; rst; stop; dow orsc_ctp6_play_fe/payload.elf && run" | xmd

####
# After this point, you can use VME pc to read, write and play data to CTP6/CTP7/MP7
#
#
# oRSCPokeKintexAddress <Address> <value>
# oRSCDumpKintexAddress <Address> <Length>
# oRSCPlayback -v --ones --do
#
####


###
# For the capture on the CTP6 side \
# Setup these registers using xmd

connect mb mdm -debugdevice deviceNr 2
mwr 0x600F0034 0x7c  ## Sets up the Orbit Char
mwr 0x600F0038 0x1   ## MB capture trigger
mwr 0x600F0038 0x0   ## MB capture trigger

mrd 0x600F005c 48  # You should see the links locked

# To READ the RAMs on CTP6 and display data,
# USE xmd
mrd 0x60000000 1024
mrd 0x60001000 1024
mrd 0x60002000 1024
....

