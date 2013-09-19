/*
 * oRSC_bbmain.c
 *
 *  Created on: Jul 7, 2013
 *      Author: tgorski
 */
#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xintc.h"
#include "xbasic_types.h"
#include "xgpio.h"
#include "i2cutils.h"
#include "ecl_config.h"

typedef struct {
  u8 delayval;
  int fieldID;
} delay_tbl_entry_t;

delay_tbl_entry_t delay_tbl[8] = {
  {0x00, DELAY_CK160IN},
  {0x10, DELAY_RSTIN},
  {0x10, DELAY_BX0IN},
  {0x10, DELAY_L1AIN},
  {0xA0, DELAY_CK80OUT},
  {0x70, DELAY_RSTOUT},
  {0x70, DELAY_BX0OUT},
  {0x70, DELAY_L1AOUT} };


int main(void) {
  int i1;
  xil_printf("\r\noRSC Setup Test Program.  07JUL13\r\n");

  ecl_config_init();
  init_DS25CP104();

  // initialize delays from table
  for (i1=0; i1<8; i1++)
    ecl_set_delay(delay_tbl[i1].delayval, delay_tbl[i1].fieldID);
    ecl_set_mux_field(CLK, 2);
    ecl_set_mux_field(RST, 2);
    ecl_set_mux_field(BX0, 2);
    ecl_set_mux_field(L1A, 2);


    reset_clk_synths();
    init_SI5324A();
    check_SI5324A();
    init_SI5324C();
    check_SI5324C();



  while(1) {

	for (i1=0; i1<1000; i1++);
  }

  return 0;
}
