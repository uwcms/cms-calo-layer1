/*
 * =====================================================================================
 *
 *       Filename:  OrscEchoEmulator.h
 *
 *    Description:  Emulate the oRSC, but just echo back the byte stream.
 *
 *         Author:  Austin Belknap, Evan Friis (UW Madison)
 *
 * =====================================================================================
 */


#ifndef OrscEchoEmulator_h
#define OrscEchoEmulator_h

#include "OrscEmulator.h"

class OrscEchoEmulator : public OrscEmulator {
  public:
    OrscEchoEmulator();
    void doStuff();
    static VMEController* getVMEController()
    {
      return (VMEController*) getOrscEchoEmulator();
    }
    static OrscEchoEmulator* getOrscEchoEmulator();
};


#endif
