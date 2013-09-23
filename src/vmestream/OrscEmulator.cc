#include "OrscEmulator.h"


OrscEmulator::OrscEmulator()
{
    register1 = (uint32_t*) malloc(sizeof(uint32_t));
    register2 = (uint32_t*) malloc(sizeof(uint32_t));
    ram1      = (uint32_t*) malloc(sizeof(uint32_t));
    ram2      = (uint32_t*) malloc(sizeof(uint32_t));
}


OrscEmulator::~OrscEmulator()
{
    free(register1);
    free(register2);
    free(ram1);
    free(ram2);
}


OrscEmulator*
OrscEmulator::getOrscEmulator()
{
    OrscEmulator* out = new OrscEmulator();
    return out;
}


bool
OrscEmulator::read(unsigned long address, size_t size, void* value)
{
    assert(size == 4); // assume 32-bit for now

    switch(address)
    {
        case 0xBEEFCAFE:
            memcpy(value, register1, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            memcpy(value, register2, sizeof(uint32_t));
            break;
        case 0xCAFEBABE:
            memcpy(value, ram1, sizeof(uint32_t));
            break;
        case 0xFACEFEED:
            memcpy(value, ram2, sizeof(uint32_t));
            break;
        default:
            return 0;
    }
    return 1;
}


bool
OrscEmulator::write(unsigned long address, size_t size, void* value)
{
    assert(size == 4); // assume 32-bit for now

    switch(address)
    {
        case 0xBEEFCAFE:
            memcpy(register1, value, sizeof(uint32_t));
            break;
        case 0xDEADBEEF:
            memcpy(register2, value, sizeof(uint32_t));
            break;
        case 0xCAFEBABE:
            memcpy(ram1, value, sizeof(uint32_t));
            break;
        case 0xFACEFEED:
            memcpy(ram2, value, sizeof(uint32_t));
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
OrscEmulator::reset()
{
    return 1;
}
