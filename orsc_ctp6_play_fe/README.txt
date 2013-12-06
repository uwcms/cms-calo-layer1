### ### ### ### ### ### ### ### ### 
#  oRSC BE and FE code explanation \
#  This code is responsible for: \
#
#  Write patterns to Kintex RAMs on oRSC \
#  patterns generated at VME pc are \
#  transferred and written to BE RAMs \
#  This code transfers and writes the pattern \
#  to FE (Kintex) RAMs on oRSC
#
### ### ### ### ### ### ### ### ### 


## Configure and Connect to BE via JTAG

fpga -f bitfiles/orsc/top_be.bit -debugdevice deviceNr 1
connect mb mdm -debugdevice deviceNr 1
dow orsc_ctp6_play_be/payload.elf 

## Configure, Connect and RUN FE via JTAG

fpga -f bitfiles/orsc/top_fe.bit -debugdevice deviceNr 2
connect mb mdm -debugdevice deviceNr 2
dow orsc_ctp6_play_fe/payload.elf 
run

# Then run the BE code on xmd prompt
run

# This enables the uart on BE and FE to start \
# listening to VME PC.
# Now run the VME code that will generate \
# patterns and send to BE RAMs
# In about 3-3.5 mins. all 23 links on oRSC FE 
# should be filled

# After the entire data is played by VME code \
# execute stop on the BE and FE xmd prompt

stop

# Then you can read the memories written on oRSC FE
# e.g. 

mrd 0x10000000 256

# We will add more information how to play these data \
# to CTP6 soon...
