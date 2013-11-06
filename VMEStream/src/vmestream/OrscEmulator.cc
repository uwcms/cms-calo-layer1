#include "OrscEmulator.h"

#include <stdlib.h>

#include "VMEStreamAddress.h"

OrscEmulator::OrscEmulator()
{
    local_send_size  = 0;
    local_recv_size  = 0;
    remote_send_size = 0;
    remote_recv_size = 0;
    recv_data = (uint32_t*) calloc(VMERAMSIZE, sizeof(uint32_t));
    send_data = (uint32_t*) calloc(VMERAMSIZE, sizeof(uint32_t));

    // setup the VME stream mechanism
    input_buffer = cbuffer_new();
    output_buffer = cbuffer_new();

    stream = vmestream_initialize_mem(
            input_buffer,
            output_buffer,
            &local_send_size,      // local_send_size
            &local_recv_size,      // local_recv_size
            &remote_send_size,     // remote_send_size
            &remote_recv_size,     // remote_recv_size
            recv_data,             // recv_data
            send_data,             // send_data
            VMERAMSIZE);
}


OrscEmulator::~OrscEmulator()
{
    free(recv_data);
    free(send_data);
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
        case ORSC_RECV_SIZE:
            assert(size == 4);
            memcpy(value, &local_recv_size, sizeof(uint32_t));
            break;
        case ORSC_SEND_SIZE:
            assert(size == 4);
            memcpy(value, &local_send_size, sizeof(uint32_t));
            break;
        default:
            exit(10);
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
        case PC_RECV_SIZE:
            assert(size == 4);
            memcpy(&remote_recv_size, value, sizeof(uint32_t));
            break;
        case PC_SEND_SIZE:
            assert(size == 4);
            memcpy(&remote_send_size, value, sizeof(uint32_t));
            break;
        default:
            exit(11);
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
        case ORSC2PC_DATA:
            assert(datawidth == 4);
            memcpy(buffer, send_data, n_bytes);
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
        case PC2ORSC_DATA:
            assert(datawidth == 4);
            memcpy(recv_data, buffer, n_bytes);
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
