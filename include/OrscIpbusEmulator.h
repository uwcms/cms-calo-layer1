/*
 * =====================================================================================
 *
 *       Filename:  OrscIpbusEmulator.h
 *
 *    Description:  Emulate the oRSC serving IPBus packets.
 *
 *         Author:  Austin Belknap, Evan Friis (UW Madison)
 *
 * =====================================================================================
 */

#ifndef OrscIpbusEmulator_h
#define OrscIpbusEmulator_h

#include "OrscEmulator.h"

class OrscIpbusEmulator : public OrscEmulator {
  public:
    OrscIpbusEmulator();
    void doStuff();
    static VMEController* getVMEController()
    {
      return (VMEController*) getOrscIpbusEmulator();
    }
    static OrscIpbusEmulator* getOrscIpbusEmulator();
};


#endif
