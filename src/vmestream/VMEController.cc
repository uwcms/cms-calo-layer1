#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "VMEController.h"
#include "OrscEmulator.h"
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
        else if (strcmp(environment, "TEST") == 0) {
            std::cout << "Emulating oRSC" << std::endl;
            return OrscEmulator::getVMEController();
        }
    }

    return null::getVMEController();
}
