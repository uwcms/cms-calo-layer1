#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <cstring>
using std::cout;
using std::cerr;
using std::endl;
using std::hex;

#include "caen.h"

#include "CAENVMEtypes.h"

extern "C" {
#include "CAENVMElib.h"
}

caen* caen::theInstance = 0;

void
caen::done()
{
  if(theInstance != 0) {
    delete theInstance;
    theInstance = 0;
  }
}

caen*
caen::getCAEN()
{
  if(theInstance == 0)
    {
      theInstance = new caen();
      atexit(done);
    }
  return theInstance;
}

caen::caen()
{
  type = 0;
  CVBoardTypes boardType = cvV2718;
  //short link = 0;
  //short boardNumber = 0;
  // Install read of environment variable CAEN_LINK
  char *caen_link = getenv("CAEN_LINK");
  char *caen_board = getenv("CAEN_BOARD"); 
  short link;
  short boardNumber; 
  if(caen_link!=NULL){
    if(strcmp(caen_link,"1")==0){
      link = 1;
    }else if(strcmp(caen_link,"2")==0){
      link = 2;
    }else if(strcmp(caen_link,"3")==0){ // Necessary for the PCIe with 4 links - added 5.6.2013 - Pam
      link = 3;
    }else{
      link = 0;     // default to 0
    }
  }else{
    link = 0; // no environment variable
  }
  if(caen_board!=NULL){
    
    if(strcmp(caen_board,"1")==0){
      boardNumber = 1;
    }else if(strcmp(caen_board,"2")==0){
      boardNumber = 2;
    }else if(strcmp(caen_board,"3")==0){
      boardNumber = 3;
    }else if(strcmp(caen_board,"4")==0){
      boardNumber = 4;
    }else if(strcmp(caen_board,"5")==0){
      boardNumber = 5;
    }else if(strcmp(caen_board,"6")==0){
      boardNumber = 6;
    }else if(strcmp(caen_board,"7")==0){
      boardNumber = 7;
    }else{
      boardNumber = 0;   // default to 0
    }
  }else{
    boardNumber = 0; // no environment variable
  }
  cout << "Booting from CAEN link and board " 
       << link << " " << boardNumber << endl;
  status = CAENVME_Init(boardType, link, boardNumber, &handle);
  if(status != cvSuccess)
    {
      cerr << "CAEN module failed to initialize - exiting" << endl;
      exit(10);
    }
  sleep(1);
  CAENVME_SetFIFOMode(handle, 0);
}

caen::~caen()
{
  /* Close the devices */
  status = CAENVME_End(handle);
  if(status != cvSuccess)
    {
      cerr << "CAEN module failed to close properly" << endl;
    }
}

bool
caen::reset()
{
  CVErrorCodes status = CAENVME_SystemReset(handle);
  if(status != cvSuccess)
    {
      return false;
    }
  return true;
}

bool
caen::read(unsigned long address, size_t size, void* value)
{

  CVAddressModifier am = cvA32_U_DATA;
  if(type == 0) am = cvA32_U_DATA;
  else if(type == 1) am = cvA24_U_DATA;
  else if (type == 2) am = cvA16_U;

  CVDataWidth dw = cvD32;
  if(size == 1) dw = cvD8;
  else if(size == 2) dw = cvD16;
  else if(size == 4) dw = cvD32;
  else if(size == 8) dw = cvD64;

  int stat = cvSuccess;
  stat = CAENVME_ReadCycle(handle, address, value, am, dw);
  if (stat != cvSuccess) 
    {
      /*
	for(int i = 0; i < 100; i++)
	{
	sleep(1);
	cout << "Handle            = " << hex << handle << endl;
	cout << "Address           = " << hex << address << endl;
	cout << "Size              = " << hex << size << endl;
	cout << "Value (address)   = " << hex << value << endl;
	cout << "Value (data)      = " << hex << *((unsigned long *) value) << endl;
	cout << "CVAddressModifier = " << hex << am << endl;
	cout << "CVDataWidth       = " << hex << dw << endl;
	cout << "status            = " << hex << stat << endl;
	cout << "i                 = " << hex << i << endl;
      */
      cerr << "CAEN could not read " << hex << address 
	   << "status = " << stat << endl;
      return false;
    }
  return true;
}

bool
caen::write(unsigned long address, size_t size, void* value)
{

  CVAddressModifier am = cvA32_U_DATA;
  if(type == 0) am = cvA32_S_DATA;
  else if(type == 1) am = cvA24_S_DATA;
  else if (type == 2) am = cvA16_S;

  CVDataWidth dw = cvD32;
  if(size == 1) dw = cvD8;
  else if(size == 2) dw = cvD16;
  else if(size == 4) dw = cvD32;
  else if(size == 8) dw = cvD64;

  status = CAENVME_WriteCycle(handle, address, value, am, dw);
  if (status != cvSuccess) 
    {
      cerr << "CAEN could not write " << hex << address << endl;
      return false;
    }
  return true;
}
bool
caen::multiread(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter)
{

  CVAddressModifier am = cvA32_U_DATA;
  if(type == 0) am = cvA32_U_DATA;
  else if(type == 1) am = cvA24_U_DATA;
  else if (type == 2) am = cvA16_U;

  CVDataWidth dw = cvD32;
  if(size == 1) dw = cvD8;
  else if(size == 2) dw = cvD16;
  else if(size == 4) dw = cvD32;
  else if(size == 8) dw = cvD64;


  uint32_t Addrs[dataCounter];
  uint32_t Buffer[dataCounter];
  CVAddressModifier AMs[dataCounter];
  CVDataWidth DWs[dataCounter];
  CVErrorCodes ECs[dataCounter];

  for(int i=0; i<dataCounter; i++)
  {
    Addrs[i]=addresses[i];
    AMs[i]=am;
    DWs[i]=dw;
  }

  int stat = cvSuccess;
  stat = CAENVME_MultiRead(handle, Addrs, Buffer, dataCounter, AMs, DWs,ECs);
  if (stat != cvSuccess) 
    {
      /*
	for(int i = 0; i < 100; i++)
	{
	sleep(1);
	cout << "Handle            = " << hex << handle << endl;
	cout << "Address           = " << hex << address << endl;
	cout << "Size              = " << hex << size << endl;
	cout << "Value (address)   = " << hex << value << endl;
	cout << "Value (data)      = " << hex << *((unsigned long *) value) << endl;
	cout << "CVAddressModifier = " << hex << am << endl;
	cout << "CVDataWidth       = " << hex << dw << endl;
	cout << "status            = " << hex << stat << endl;
	cout << "i                 = " << hex << i << endl;
      */
      cerr << "CAEN could not block read " << endl;
      return false;
    }
  for(int i=0; i<dataCounter; i++)
  {
    data[i]=Buffer[i];
  }
  return true;
}

bool
caen::multiwrite(unsigned int *addresses, size_t size, unsigned short *data, int dataCounter)
{

  CVAddressModifier am = cvA32_U_DATA;
  if(type == 0) am = cvA32_S_DATA;
  else if(type == 1) am = cvA24_S_DATA;
  else if (type == 2) am = cvA16_S;

  CVDataWidth dw = cvD32;
  if(size == 1) dw = cvD8;
  else if(size == 2) dw = cvD16;
  else if(size == 4) dw = cvD32;
  else if(size == 8) dw = cvD64;

  uint32_t Addrs[dataCounter];
  uint32_t Buffer[dataCounter];
  CVAddressModifier AMs[dataCounter];
  CVDataWidth DWs[dataCounter];
  CVErrorCodes ECs[dataCounter];

  for(int i=0; i<dataCounter; i++)
  {
    Addrs[i]=addresses[i];
    Buffer[i]=data[i];
    AMs[i]=am;
    DWs[i]=dw;
  }

  status = CAENVME_MultiWrite(handle, Addrs, Buffer, dataCounter, AMs, DWs, ECs);
  if (status != cvSuccess) 
    {
      cerr << "CAEN could not write data block" << endl;
      return false;
    }
  return true;
}
