#include "OrscEchoEmulator.h"

// setup is handled by base class
OrscEchoEmulator::OrscEchoEmulator() : OrscEmulator() {}

// Echo things from ram1->ram2, respecting the VMEStream protocol
void
OrscEchoEmulator::doStuff() {
  // move from local memory into buffers
  
        // printf("oRSC before transfer\n");
        // printf("  remote_recv_size: %d\n", *(stream->remote_recv_size));
        // printf("  remote_send_size: %d\n", *(stream->remote_send_size));
        // printf("  local_recv_size:  %d\n", *(stream->local_recv_size));
        // printf("  local_send_size:  %d\n", *(stream->local_send_size));
  vmestream_transfer_data(stream);
        // printf("oRSC after transfer\n");
        // printf("  remote_recv_size: %d\n", *(stream->remote_recv_size));
        // printf("  remote_send_size: %d\n", *(stream->remote_send_size));
        // printf("  local_recv_size:  %d\n", *(stream->local_recv_size));
        // printf("  local_send_size:  %d\n", *(stream->local_send_size));
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
