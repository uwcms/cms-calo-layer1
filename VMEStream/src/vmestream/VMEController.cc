#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "VMEController.h"
#include "OrscEchoEmulator.h"
#include "OrscIpbusEmulator.h"
#include "caen.h"
#include "null.h"

VMEController*
VMEController::getVMEController()
{
    char *environment = getenv("VME_CONTROLLER");
    std::cout << "VME controller:" << environment << std::endl;

    if(environment != 0)
    {
        if(strcmp(environment, "CAEN") == 0) {
            return caen::getVMEController();
        }
        else if (strcmp(environment, "TESTECHO") == 0) {
            std::cout << "Emulating oRSC with Echo" << std::endl;
            return OrscEchoEmulator::getVMEController();
        }
        else if (strcmp(environment, "TESTIPBUS") == 0) {
            std::cout << "Emulating oRSC as an IPBus Server" << std::endl;
            return OrscIpbusEmulator::getVMEController();
        }
        else {
            std::cerr << "VME_CONTROLLER not set properly!" << std::endl;;
            std::cerr << 
                "-> VME_CONTROLLER = [CAEN, TESTECHO, TESTIPBUS]" << std::endl;
            exit(1);
        }
    }

    return null::getVMEController();
}
