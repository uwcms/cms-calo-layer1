#include "OrscEchoEmulator.h"

// setup is handled by base class
OrscEchoEmulator::OrscEchoEmulator() : OrscEmulator() {}

// Echo things from ram1->ram2, respecting the VMEStream protocol
void
OrscEchoEmulator::doStuff() {
  // move from local memory into buffers
  vmestream_transfer_data(stream);
  // now echo the data
  while (cbuffer_size(output_buffer) && cbuffer_freespace(input_buffer)) {
    cbuffer_push_back(input_buffer, cbuffer_pop_front(output_buffer));
  }
}

OrscEchoEmulator*
OrscEchoEmulator::getOrscEchoEmulator()
{
    OrscEchoEmulator* out = new OrscEchoEmulator();
    return out;
}
