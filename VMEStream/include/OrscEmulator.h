#ifndef _OrscEmulator_h
#define _OrscEmulator_h


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "VMEStream.h"
#include "circular_buffer.h"

#include "VMEController.h"

class OrscEmulator : public VMEController
{
    protected:

        uint32_t register1;  // PC_RECV_SIZE
        uint32_t register2;  // PC_SEND_SIZE
        uint32_t register3;  // ORSC_RECV_SIZE
        uint32_t register4;  // ORSC_SEND_SIZE
        uint32_t* ram1;      // PC2ORSC_DATA
        uint32_t* ram2;      // ORSC2PC_DATA

        VMEStream* stream;
        CircularBuffer* input_buffer;
        CircularBuffer* output_buffer;

    public:
        OrscEmulator();
        virtual ~OrscEmulator();

        virtual bool reset();
        virtual bool read(unsigned long address, size_t size, void* value);
        virtual bool write(unsigned long address, size_t size, void* value);
        virtual bool multiread(unsigned int *addresses, size_t size,
                unsigned short *data, int dataCounter);
        virtual bool multiwrite(unsigned int *addresses, size_t size,
                unsigned short *data, int dataCounter);
        virtual bool block_read(uint32_t address, size_t datawidth,
                void* buffer, size_t n_bytes);
        virtual bool block_write(uint32_t address, size_t datawidth,
                void* buffer, size_t n_bytes);

        // does nothing, see subclasses for specific emulation functionality
        virtual void doStuff()=0;
};

#endif
