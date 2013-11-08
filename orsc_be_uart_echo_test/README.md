### oRSC_(fe|be)_uart_echo_test

- Setup the BE UART serial port to the VME PC
- After connecting the cable from oRSC to the VME PC
dmesg 

- At the end of the log it should show up the USB port that has the UART device
- e.g. /dev/ttyUSB0
- Run screen to listen to this port

screen /dev/ttyUSB0  # Keep it running

- Configure and Connect to BE via JTAG

fpga -f /afs/hep.wisc.edu/ecad/mathias/orsc_bits/top_be.bit
connect mb mdm -debugdevice deviceNr 1
dow orsc_be_uart_echo_test/payload.elf 

- Configure and Connect to FE via JTAG to listen to BE
fpga -f /afs/hep.wisc.edu/ecad/mathias/orsc_bits/top_fe.bit
connect mb mdm -debugdevice deviceNr 2
dow orsc_fe_uart_echo_test/payload.elf 
run
read_uart

- Then run the BE
run

- On the terminal running screen, the SUCESS should be printed out 
 XST_SUCCESS

