#include "OrscEmulator.h"
#include <stdlib.h>

// we use 512 words a time for the RAMS
#define VMERAMSIZE (512 * sizeof(uint32_t))

OrscEmulator::OrscEmulator()
{
    register1 = 0;
    register2 = 0;
    ram1      = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));
    ram2      = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    // setup the VME stream mechanism
    input_buffer = cbuffer_new();
    output_buffer = cbuffer_new();

    stream = (VMEStream*)malloc(sizeof(VMEStream));
    stream->MAXRAM = VMERAMSIZE;
    stream->input = input_buffer;
    stream->output = output_buffer;
    stream->tx_size = &register2;
    stream->rx_size = &register1;
    stream->tx_data = ram2;
    stream->rx_data = ram1;
}


OrscEmulator::~OrscEmulator()
{
    free(ram1);
    free(ram2);
    cbuffer_free(input_buffer);
    cbuffer_free(output_buffer);
    vmestream_destroy(stream);
}


bool
OrscEmulator::read(unsigned long address, size_t size, void* value)
{

    switch(address)
    {
        case 0xBEEFCAFE:
            assert(size == 4);
            memcpy(value, &register1, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            assert(size == 4);
            memcpy(value, &register2, sizeof(uint32_t));
            break;
        case 0xCAFEBABE:
            memcpy(value, ram1, size * sizeof(uint32_t));
            break;
        case 0xFACEFEED:
            memcpy(value, ram2, size * sizeof(uint32_t));
            break;
        default:
            return 0;
    }
    return 1;
}


bool
OrscEmulator::write(unsigned long address, size_t size, void* value)
{
    switch(address)
    {
        case 0xBEEFCAFE:
            assert(size == 4);
            memcpy(&register1, value, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            assert(size == 4);
            memcpy(&register2, value, sizeof(uint32_t));
            break;
        case 0xCAFEBABE:
            memcpy(ram1, value, size * sizeof(uint32_t));
            break;
        case 0xFACEFEED:
            memcpy(ram2, value, size * sizeof(uint32_t));
            break;
        default:
            return 0;
    }
    return 1;
}


bool
OrscEmulator::multiread(unsigned int *addresses, size_t size,
        unsigned short *data, int datacounter)
{
    return 1;
}


bool
OrscEmulator::multiwrite(unsigned int *addresses, size_t size,
        unsigned short *data, int datacounter)
{
    return 1;
}


bool
OrscEmulator::block_read(uint32_t address, size_t datawidth, void* buffer, size_t n_bytes)
{
    return 1;
}

bool
OrscEmulator::block_write(uint32_t address, size_t datawidth, void* buffer, size_t n_bytes)
{
    return 1;
}

bool
OrscEmulator::reset()
{
    return 1;
}
