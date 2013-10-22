#ifndef tracer_h
#define tracer_h

/*
 * The tracer allows specification of a fixed address
 * to store debug flags.  If there are consecutive addresses
 * available, one can set slots > 1, which will store the 
 * history of the traced commands.
 *
 * One can the read out these status flags (and trace the
 * program flow) by reading the addresses using xmd.
 */

#include <stdint.h>

// set the base address and the number of history slots
void setup_tracer(uint32_t* base, unsigned int slots);

// Set the trace memory to the flag, and save previous 
// ones in the history.
void set_trace_flag(uint32_t flag);

#endif
