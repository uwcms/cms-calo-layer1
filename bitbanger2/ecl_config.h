/*
 * ecl_config.h
 *
 *  Created on: Jul 7, 2013
 *      Author: tgorski
 */

#ifndef ECL_CONFIG_H_
#define ECL_CONFIG_H_

#include "xil_types.h"
// file of config functions for ECL section

#define BB_WSTB_CK160IN		(1 << 0)
#define BB_WSTB_RSTIN		(1 << 1)
#define BB_WSTB_BX0IN		(1 << 2)
#define BB_WSTB_L1AIN		(1 << 3)
#define BB_WSTB_CK80OUT		(1 << 4)
#define BB_WSTB_RSTOUT		(1 << 5)
#define BB_WSTB_BX0OUT		(1 << 6)
#define BB_WSTB_L1AOUT		(1 << 7)
#define BB_SHFTREGRST		(1 << 8)
#define BB_SHFTREGDATA		(1 << 9)
#define BB_DELAYREGCLK		(1 << 10)
#define BB_MUXREGCLK		(1 << 11)
#define BB_AVAGORST_B		(1 << 12)
#define BB_CLKA_RST_B		(1 << 13)
#define BB_CLKC_RST_B		(1 << 14)

#define DELAY_CK160IN		(0)
#define DELAY_RSTIN			(1)
#define DELAY_BX0IN			(2)
#define DELAY_L1AIN			(3)
#define DELAY_CK80OUT		(4)
#define DELAY_RSTOUT		(5)
#define DELAY_BX0OUT		(6)
#define DELAY_L1AOUT		(7)

#define ECLMX_CLKS0			(1 << 0)
#define ECLMX_CLKS1			(1 << 1)
#define ECLMX_RSTS0			(1 << 2)
#define ECLMX_RSTS1			(1 << 3)
#define ECLMX_BX0S0			(1 << 4)
#define ECLMX_BX0S1			(1 << 5)
#define ECLMX_L1AS0			(1 << 6)
#define ECLMX_L1AS1			(1 << 7)

typedef enum {CLK=0, RST, BX0, L1A} mux_reg_field_t;

void ecl_config_init(void);
void ecl_set_delay_raw(u8 delayval, u16 stbmask);
void ecl_set_muxreg_raw(u8 regval);

void ecl_set_delay(u8 delayval, int index);
u8 ecl_get_delay(int index);
void ecl_set_mux_field(mux_reg_field_t field, u8 val);
u8 ecl_get_mux_field(mux_reg_field_t field);

void reset_clk_synths(void);

#endif /* ECL_CONFIG_H_ */
