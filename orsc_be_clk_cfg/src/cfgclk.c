/*
 * Configuration of CLKs and LEDs on the BE
 *
 * Author: Tapas R. Sarangi, UW Madison
 *
 *
 */
#include "platform.h"
#include "xparameters.h"        
#include "xuartlite.h"
#include "xintc.h"		
#include "xil_exception.h"
#include "macrologger.h"
#include "tracer.h"
#include "i2cutils.h"
#include "xio.h"
#include "addr.h"
#include "flashled.h"
#include <stdio.h>

/*  STDOUT functionality  */

int main(void) {

  XIo_Out32(0x10008044,0x1);  // UnReset Clock A
  XIo_Out32(0x10008048,0x1);  // UnReset Clock C

  init_platform();
  init_DS25CP104();

  /// Delay the signals to sync them later
  int i1;
  for(i1=0; i1<1000000; i1++);

  init_SI5324A();
  check_SI5324A();
  init_SI5324C();
  check_SI5324C();  
  
  /*   Set the REG_DONECFG value to 1  */
  /*   so that FE can be configured */
  XIo_Out32(REG_DONECFG, 0x1);

  /* Configure LEDs */
  while(1){
    red_turnon();
    for(i1=0; i1<12; i1++)
      xil_printf("Go GREEN....\n\r");
    green_turnon();
    for(i1=0; i1<12; i1++)
      xil_printf("GREEN AWAY...\n\r");
  }
  
  return 0;
}
