#include "OrscEmulator.h"

// we use 512 words a time for the RAMS
#define MAXRAM (512 * sizeof(uint32_t))

OrscEmulator::OrscEmulator()
{
    register1 = 0;
    register2 = 0;
    ram1      = (uint32_t*) malloc(MAXRAM);
    ram2      = (uint32_t*) malloc(MAXRAM);
}


OrscEmulator::~OrscEmulator()
{
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

// Echo things from ram1->ram2, respecting the VMEStream protocol
void
OrscEmulator::doStuff() {
  // input buffer has data and output is empty
  if (register1 && !register2) {
    memcpy(ram2, ram1, register1 * sizeof(uint32_t));
    register2 = register1;
    register1 = 0;
  }
}


bool
OrscEmulator::reset()
{
    return 1;
}
