#include "tracer.h"

static uint32_t* tracer_base;
static unsigned int tracer_slots;

void setup_tracer(uint32_t* base, unsigned int slots) {
    tracer_base = base;
    tracer_slots = slots;
}

void set_trace_flag(uint32_t flag) {
    // move back the history
    for (int i = 0; i < tracer_slots - 1; ++i) {
        tracer_base[tracer_slots - 1 - i] = tracer_base[tracer_slots - 1 - i - 1];
    }
    tracer_base[0] = flag;
}
