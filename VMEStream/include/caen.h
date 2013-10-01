#ifndef caen_h
#define caen_h

#include <stdint.h>
#include "VMEController.h"
#include "CAENVMEtypes.h"

class caen : public VMEController
{
 public:
  static VMEController* getVMEController()
    {
      return (VMEController*) getCAEN();
    }
  static caen* getCAEN();
  static void done();
  virtual bool reset();
  virtual bool read(unsigned long address, size_t size, void* value);
  virtual bool write(unsigned long address, size_t size, void* value);
  virtual bool multiread(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter);
  virtual bool multiwrite(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter);

  //MG 4May07
  //long getHandle() {return handle;}
  int32_t getHandle() {return handle;}

 private:
  // Private to make this a singleton
  caen();
  virtual ~caen();
  // Copy constructor and equality operator are not implemented
  caen(const caen&);
  const caen& operator=(const caen&);
  static caen* theInstance;
  // Data
  // MG 4May07
  //long handle;
  int32_t handle;
  CVErrorCodes status;
};

#endif
