#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "VMEController.h"
#include "caen.h"
#include "null.h"

VMEController*
VMEController::getVMEController()
{
  char *environment = getenv("VME_CONTROLLER"); 
  
  if(environment != 0)
  {
    if(strcmp(environment, "CAEN") == 0)
	  return caen::getVMEController();
  }
  
  return null::getVMEController();
}
