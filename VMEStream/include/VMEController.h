#ifndef VMEController_h
#define VMEController_h

#include <iostream>
using std::endl;
using std::cerr;

class VMEController
{
 public:

  static VMEController* getVMEController();
  virtual ~VMEController(){};

  void setA32() {type = 0;}
  void setA24() {type = 1;}
  void setA16() {type = 2;}
  virtual bool reset() = 0;
  virtual bool read(unsigned long address, size_t size, void* value) = 0;
  virtual bool write(unsigned long address, size_t size, void* value) = 0;
  virtual bool multiread(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter) = 0;
  virtual bool multiwrite(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter) = 0;

  bool write(unsigned long address, unsigned long value)
    {
      return write(address, 4, (void*) &value);
    }
  bool write(unsigned long address, unsigned short value)
    {
      return write(address, 2, (void*) &value);
    }
  bool write(unsigned long address, unsigned char value)
    {
      return write(address, 1, (void*) &value);
    }

  // Do some stuff. This function does no stuff
  // unless it is overridden by a derived class to do some
  // emulation or other business.
  virtual void doStuff() {}

 protected:

  int type;                  /* Currently used type */

};

#endif

