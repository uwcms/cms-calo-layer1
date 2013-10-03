// Changed: 14 June 2007, Christos Laz.
// Reason : The read/write/reset messages are now printed 10 times only
#ifndef null_h
#define null_h

#include "VMEController.h"
// using log4cplus
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <iomanip>

using namespace log4cplus;
// using log4cplus
#include <iomanip>

class null : public VMEController
{
 public:
  static VMEController* getVMEController() { return (VMEController*) getNULL(); }
  static null* getNULL()
    {
      if(theInstance == 0)
	{
	  theInstance = new null();
	}
      return theInstance;
    }

  virtual bool reset() {
	Logger logger = Logger::getInstance("VMEController::null");
	static int rst_counter=0;
	if (rst_counter<10)
	{
		LOG4CPLUS_WARN(logger, "null::reset() called");
		//cerr << "null::reset() called" << endl;
		rst_counter++;
	}
        return true;
  }

  virtual bool read(unsigned long address, size_t size, void* value)
  {
        Logger logger = Logger::getInstance("VMEController::null");

	static int read_counter=0;
	if (read_counter<10)
	{
		LOG4CPLUS_WARN(logger, "null::read() called");
		//cerr << "null::read() called" << endl;
		read_counter++;
	}
        return true;
  }
  virtual bool write(unsigned long address, size_t size, void* value)
  {
        Logger logger = Logger::getInstance("VMEController::null");

	static int write_counter=0;
	if (write_counter<10)
	{
		LOG4CPLUS_WARN(logger, "null::write() called");
		//cerr << "null::write() called" << endl;
		write_counter++;
	}
        return true;
  }

  virtual bool multiwrite(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter)
  {
        Logger logger = Logger::getInstance("VMEController::null");

	static int write_counter=0;
	if (write_counter<10)
	{
		LOG4CPLUS_WARN(logger, "null::write() called");
		//cerr << "null::write() called" << endl;
		write_counter++;
	}
        return true;
  }

  virtual bool multiread(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter)
  {
        Logger logger = Logger::getInstance("VMEController::null");

	static int read_counter=0;
	if (read_counter<10)
	{
		LOG4CPLUS_WARN(logger, "null::read() called");
		//cerr << "null::read() called" << endl;
		read_counter++;
	}
        return true;
  }

  virtual bool block_read(uint32_t address, size_t datawidth, void* buffer, size_t n_bytes)
  {
      Logger logger = Logger::getInstance("VMEController::null");

      static int read_counter = 0;
      if (read_counter < 10) {
          LOG4CPLUS_WARN(logger, "null::block_read() called");
          read_counter++;
      }
      return true;
  }

  virtual bool block_write(uint32_t address, size_t datawidth, void* buffer, size_t n_bytes)
  {
      Logger logger = Logger::getInstance("VMEController::null");

      static int read_counter = 0;
      if (read_counter < 10) {
          LOG4CPLUS_WARN(logger, "null::block_write() called");
          read_counter++;
      }
      return true;
  }



 private:
  // Private to make this a singleton
  null()
{
        Logger logger = Logger::getInstance("VMEController::null");
	LOG4CPLUS_WARN(logger, "null::null() called - no VME activity will happen");
	LOG4CPLUS_WARN(logger, "*** read/write/reset messages will be printed only 10 times ***");
}

  virtual ~null() {;}

  // Copy constructor and equality operator are not implemented
  null(const null&);
  const null& operator=(const null&);
  bool device_setup_finished;
  static null* theInstance;
};

null* null::theInstance = 0;

#endif
