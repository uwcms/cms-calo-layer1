#include "OrscIpbusEmulator.h"

#include "transactionhandler.h"

// setup is handled by base class
OrscIpbusEmulator::OrscIpbusEmulator() : OrscEmulator() {}

// Ipbus things from ram1->ram2, respecting the VMEStream protocol
void
OrscIpbusEmulator::doStuff() {
  // move from local memory into buffers
  vmestream_transfer_data(stream);
  // now process the IPBus data
  handle_transaction_stream(output_buffer, 0, input_buffer);
  // move from local memory into buffers
  vmestream_transfer_data(stream);
}

OrscIpbusEmulator*
OrscIpbusEmulator::getOrscIpbusEmulator()
{
    OrscIpbusEmulator* out = new OrscIpbusEmulator();
    return out;
}
