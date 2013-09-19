/*
 * ecl_config.c
 *
 *  Created on: Jul 7, 2013
 *      Author: tgorski
 */

#include <xparameters.h>
#include <xgpio.h>
#include <stdio.h>
#include <xstatus.h>
#include "ecl_config.h"

inline void setgpiobits(u8 val, u16 mask);

XGpio GpioOutput; /* The driver instance for GPIO Device configured as O/P */

static u16 gpio_state;

static u8 muxstate;
static u8 delaystate[8];

void ecl_config_init(void) {
  int Status;

  Status = XGpio_Initialize(&GpioOutput, 0);
  if (Status != XST_SUCCESS)  {
	xil_printf("GPIO driver init failure!\n");
	return;
  }
  XGpio_SetDataDirection(&GpioOutput, 1, 0x0);  // set direction to outputs

  gpio_state = 0xff;
  gpio_state |= BB_DELAYREGCLK | BB_MUXREGCLK | BB_AVAGORST_B | BB_CLKA_RST_B | BB_CLKC_RST_B;
  XGpio_DiscreteWrite(&GpioOutput, 1, gpio_state);
  gpio_state |= BB_SHFTREGRST;
  XGpio_DiscreteWrite(&GpioOutput, 1, gpio_state);
  gpio_state &= ~BB_SHFTREGRST;
  XGpio_DiscreteWrite(&GpioOutput, 1, gpio_state);
  muxstate = 0;
  ecl_set_muxreg_raw(muxstate);
}

void ecl_set_delay_raw(u8 delayval, u16 stbmask) {
  int i1;

  for (i1=7; i1>=0; i1--) {
	setgpiobits((delayval & (1 << i1)), BB_SHFTREGDATA); 		// set data bit
	setgpiobits(0, BB_DELAYREGCLK);								// falling edge of clock
	setgpiobits(1, BB_DELAYREGCLK);								// rising edge of clock
  }
  setgpiobits(0, stbmask);								// falling edge of strobes
  setgpiobits(1, stbmask);								// rising edge of strobes
}


void ecl_set_muxreg_raw(u8 regval) {
  int i1;
  for (i1=7; i1>=0; i1--) {
	setgpiobits((regval & (1 << i1)), BB_SHFTREGDATA); 		// set data bit
	setgpiobits(0, BB_MUXREGCLK);								// falling edge of clock
	setgpiobits(1, BB_MUXREGCLK);								// rising edge of clock
  }
}


inline void setgpiobits(u8 val, u16 mask) {
  if (val)
	gpio_state |= mask;
  else
	gpio_state &= ~mask;
  XGpio_DiscreteWrite(&GpioOutput, 1, gpio_state);
}

void ecl_set_mux_field(mux_reg_field_t field, u8 val) {
  int fieldbase;

  switch (field) {
  case CLK:
	fieldbase = 0;
	break;
  case RST:
	fieldbase = 2;
	break;
  case BX0:
	fieldbase = 4;
	break;
  case L1A:
	fieldbase = 6;
	break;
  default:
	return;
  }

  muxstate &= ~(0x03 << fieldbase);					// zero out bits
  muxstate |= ((val & 0x03) << fieldbase);			// or on new bits
  ecl_set_muxreg_raw(muxstate);
}

u8 ecl_get_mux_field(mux_reg_field_t field) {
  int fieldbase;
  u8 retval;

  switch (field) {
  case CLK:
	fieldbase = 0;
	break;
  case RST:
	fieldbase = 2;
	break;
  case BX0:
	fieldbase = 4;
	break;
  case L1A:
	fieldbase = 6;
	break;
  default:
	return 0;
  }
  retval = ((muxstate >> fieldbase) & 0x3);
  return retval;
}

void ecl_set_delay(u8 delayval, int index) {
  if ((index < 0) || (index > 7))
	return;
  delaystate[index] = delayval;
  ecl_set_delay_raw(delayval, (1 << index));
}

u8 ecl_get_delay(int index) {
  if ((index < 0) || (index > 7))
	return 0;
  return delaystate[index];
}


void reset_clk_synths(void) {
  int i1;
  xil_printf("Resetting Clock Synthesizers...");
  setgpiobits(0, BB_CLKA_RST_B | BB_CLKC_RST_B | BB_AVAGORST_B);
  for (i1=0; i1<10000; i1++);
  setgpiobits(1, BB_CLKA_RST_B | BB_CLKC_RST_B | BB_AVAGORST_B);
  for (i1=0; i1<1000000; i1++);
  xil_printf("reset done!\r\n");
}
