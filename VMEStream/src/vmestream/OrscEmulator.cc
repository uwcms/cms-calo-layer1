#include "OrscEmulator.h"
#include <stdlib.h>

// we use 512 words a time for the RAMS
#define VMERAMSIZE 512

OrscEmulator::OrscEmulator()
{
    register1 = 0;
    register2 = 0;
    ram1      = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));
    ram2      = (uint32_t*) malloc(VMERAMSIZE * sizeof(uint32_t));

    // setup the VME stream mechanism
    input_buffer = cbuffer_new();
    output_buffer = cbuffer_new();

    stream = vmestream_initialize_mem(
            input_buffer,
            output_buffer,
            &register2,
            &register1,
            ram2,
            ram1,
            VMERAMSIZE);
}


OrscEmulator::~OrscEmulator()
{
    free(ram1);
    free(ram2);
    cbuffer_free(input_buffer);
    cbuffer_free(output_buffer);
    free(stream);
}


/**
 * Emulator read is used only for the size registers
 */
bool
OrscEmulator::read(unsigned long address, size_t size, void* value)
{
    switch(address) {
        case 0xBEEFCAFE:
            assert(size == 4);
            memcpy(value, &register1, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            assert(size == 4);
            memcpy(value, &register2, sizeof(uint32_t));
            break;
        default:
            return 0;
    }
    return 1;
}


/**
 * Emulator write is used only for the size registers
 */
bool
OrscEmulator::write(unsigned long address, size_t size, void* value)
{
    switch(address) {
        case 0xBEEFCAFE:
            assert(size == 4);
            memcpy(&register1, value, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            assert(size == 4);
            memcpy(&register2, value, sizeof(uint32_t));
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
OrscEmulator::block_read(uint32_t address, size_t datawidth,
        void* buffer, size_t n_bytes)
{
    switch(address) {
        case 0xCAFEBABE:
            assert(datawidth == 4);
            memcpy(buffer, ram1, n_bytes);
            break;
        case 0xFACEFEED:
            assert(datawidth == 4);
            memcpy(buffer, ram2, n_bytes);
            break;
        default:
            return 0;
    }
    return 1;
}

bool
OrscEmulator::block_write(uint32_t address, size_t datawidth,
        void* buffer, size_t n_bytes)
{
    switch(address) {
        case 0xCAFEBABE:
            assert(datawidth == 4);
            memcpy(ram1, buffer, n_bytes);
            break;
        case 0xFACEFEED:
            assert(datawidth == 4);
            memcpy(ram2, buffer, n_bytes);
            break;
        default:
            return 0;
    }
    return 1;
}

bool
OrscEmulator::reset()
{
    return 1;
}
