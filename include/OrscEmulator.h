#ifndef _OrscEmulator_h
#define _OrscEmulator_h


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "VMEController.h"


class OrscEmulator : public VMEController
{
    private:
        uint32_t register1; // 0xBEEFCAFE
        uint32_t register2; // 0xDEADBEEF
        uint32_t* ram1;      // 0xCAFEBABE
        uint32_t* ram2;      // 0xFACEFEED

        OrscEmulator();
        virtual ~OrscEmulator();

    public:
        static VMEController* getVMEController()
        {
            return (VMEController*) getOrscEmulator();
        }
        static OrscEmulator* getOrscEmulator();
        virtual bool reset();
        virtual bool read(unsigned long address, size_t size, void* value);
        virtual bool write(unsigned long address, size_t size, void* value);
        virtual bool multiread(unsigned int *addresses, size_t size,
                unsigned short *data, int dataCounter);
        virtual bool multiwrite(unsigned int *addresses, size_t size,
                unsigned short *data, int dataCounter);
        virtual void doStuff();
};

#endif
