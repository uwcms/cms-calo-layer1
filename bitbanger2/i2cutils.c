/*
 * i2cutils.c
 *
 *  Created on: Jul 9, 2013
 *      Author: tgorski
 */

#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xintc.h"
#include "xbasic_types.h"
#include "xgpio.h"
#include "i2cutils.h"

typedef struct {
  int regaddr;
  u8 regval;
} i2c_dev_init_entry_t;

#define ENDOFTBL			(-1)

i2c_dev_init_entry_t DS25CP104A_init_record[] = {
		{0, 0x0c},					// Synth A/C <-120MHz TTC, Out B/D<-100 MHz Osc
		{1, 0x00}, 					// no pre-emphasis on outputs
		{2, 0x00}, 					// no equalization on inputs
		{3, 0x8f},					// no power-down,
		{ENDOFTBL, 0} };

//#define clkout320
#define clkout120

#ifdef clkout320
i2c_dev_init_entry_t SI5324A_init_record[] = {
		{0, 0x14},
		{1, 0xE4},
		{2, 0xA2},
		{3, 0x15},
		{4, 0x92},
		{5, 0xAD},
		{6, 0x36},
		{7, 0x29},
		{8, 0x00},
		{9, 0xC0},
		{10, 0x00},
		{11, 0x42},
		{19, 0x29},
		{20, 0x3E},
		{21, 0xFE},
		{22, 0xDF},
		{23, 0x1F},
		{24, 0x3F},
		{25, 0x00},
		{31, 0x00},
		{32, 0x00},
		{33, 0x03},
		{34, 0x00},
		{35, 0x00},
		{36, 0x03},
		{40, 0x80},
		{41, 0x01},
		{42, 0x4F},
		{43, 0x00},
		{44, 0x00},
		{45, 0x3E},
		{46, 0x00},
		{47, 0x00},
		{48, 0x3E},
		{55, 0x03},
		{131, 0x1F},
		{132, 0x02},
		{137, 0x01},
		{138, 0x0F},
		{139, 0xFF},
		{142, 0x00},
		{143, 0x00},
		{136, 0x40},
		{ENDOFTBL, 0} };


i2c_dev_init_entry_t SI5324C_init_record[] = {
		{0, 0x14},
		{1, 0xE4},
		{2, 0xA2},
		{3, 0x15},
		{4, 0x92},
		{5, 0xAD},
		{6, 0x36},
		{7, 0x29},
		{8, 0x00},
		{9, 0xC0},
		{10, 0x00},
		{11, 0x42},
		{19, 0x29},
		{20, 0x3E},
		{21, 0xFE},
		{22, 0xDF},
		{23, 0x1F},
		{24, 0x3F},
		{25, 0x00},
		{31, 0x00},
		{32, 0x00},
		{33, 0x03},
		{34, 0x00},
		{35, 0x00},
		{36, 0x03},
		{40, 0x80},
		{41, 0x01},
		{42, 0x4F},
		{43, 0x00},
		{44, 0x00},
		{45, 0x3E},
		{46, 0x00},
		{47, 0x00},
		{48, 0x3E},
		{55, 0x03},
		{131, 0x1F},
		{132, 0x02},
		{137, 0x01},
		{138, 0x0F},
		{139, 0xFF},
		{142, 0x00},
		{143, 0x00},
		{136, 0x40},
		{ENDOFTBL, 0} };
#endif

#ifdef clkout120
i2c_dev_init_entry_t SI5324A_init_record[] = {
		{0, 0x14},
		{1, 0xE4},
		{2, 0xA2},
		{3, 0x15},
		{4, 0x92},
		{5, 0xED},
		{6, 0x2D},
		{7, 0x2A},
		{8, 0x00},
		{9, 0xC0},
		{10, 0x00},
		{11, 0x42},
		{19, 0x29},
		{20, 0x3E},
		{21, 0xFF},
		{22, 0xDF},
		{23, 0x1F},
		{24, 0x3F},
		{25, 0xE0},
		{31, 0x00},
		{32, 0x00},
		{33, 0x03},
		{34, 0x00},
		{35, 0x00},
		{36, 0x03},
		{40, 0xE0},
		{41, 0x00},
		{42, 0xF3},
		{43, 0x00},
		{44, 0x00},
		{45, 0x3C},
		{46, 0x00},
		{47, 0x00},
		{48, 0x3C},
		{55, 0x00},
		{131, 0x1F},
		{132, 0x02},
		{137, 0x01},
		{138, 0x0F},
		{139, 0xFF},
		{142, 0x00},
		{143, 0x00},
		{136, 0x40},
		{ENDOFTBL, 0} };


i2c_dev_init_entry_t SI5324C_init_record[] = {
		{0, 0x14},
		{1, 0xE4},
		{2, 0xA2},
		{3, 0x15},
		{4, 0x92},
		{5, 0xED},
		{6, 0x2D},
		{7, 0x2A},
		{8, 0x00},
		{9, 0xC0},
		{10, 0x00},
		{11, 0x42},
		{19, 0x29},
		{20, 0x3E},
		{21, 0xFF},
		{22, 0xDF},
		{23, 0x1F},
		{24, 0x3F},
		{25, 0xE0},
		{31, 0x00},
		{32, 0x00},
		{33, 0x03},
		{34, 0x00},
		{35, 0x00},
		{36, 0x03},
		{40, 0xE0},
		{41, 0x00},
		{42, 0xF3},
		{43, 0x00},
		{44, 0x00},
		{45, 0x3C},
		{46, 0x00},
		{47, 0x00},
		{48, 0x3C},
		{55, 0x00},
		{131, 0x1F},
		{132, 0x02},
		{137, 0x01},
		{138, 0x0F},
		{139, 0xFF},
		{142, 0x00},
		{143, 0x00},
		{136, 0x40},
		{ENDOFTBL, 0} };
#endif


int write_i2c_reg(u32 i2c_driver, u8 i2c_addr, u8 regaddr, u8 wrdata);
int read_i2c_reg(u32 i2c_driver, u8 i2c_addr, u8 regaddr, u8* prdvalue);



void init_DS25CP104(void) {
  i2c_dev_init_entry_t* pentry = &DS25CP104A_init_record[0];

  while (pentry->regaddr != ENDOFTBL) {
	if (!write_i2c_reg(XPAR_CLOCK_IIC_BASEADDR, DS25CP104_I2C_ADDR, (u8) pentry->regaddr, pentry->regval)) {
	  xil_printf("?Write Failure at I2C Device Address 0x%08x, reg address 0x%08x", DS25CP104_I2C_ADDR, (u8) pentry->regaddr);
	  break;
	}
	pentry++;
  }
}


void init_SI5324A(void) {
  i2c_dev_init_entry_t* pentry = &SI5324A_init_record[0];
  while (pentry->regaddr != ENDOFTBL) {
	if (!write_i2c_reg(XPAR_CLOCK_IIC_BASEADDR, SI5324A_I2C_ADDR, (u8) pentry->regaddr, pentry->regval)) {
	  xil_printf("?Write Failure at I2C Device Address 0x%08x, reg address 0x%08x", DS25CP104_I2C_ADDR, (u8) pentry->regaddr);
	  break;
	}
	pentry++;
  }
}

void check_SI5324A(void) {
  // checks read against programmed values
  u8 rdbuf[6];
  i2c_dev_init_entry_t* pentry = &SI5324A_init_record[0];

  xil_printf("Checking Programming for CLKA SI5324\r\n");
  while (pentry->regaddr != ENDOFTBL) {
	if (!read_i2c_reg(XPAR_CLOCK_IIC_BASEADDR, SI5324A_I2C_ADDR, (u8) pentry->regaddr, &rdbuf[0])) {
	  xil_printf("?Read Failure at I2C Device Address 0x%08x, reg address 0x%08x", SI5324A_I2C_ADDR, (u8) pentry->regaddr);
	  return;
	}
	if ((rdbuf[0] != pentry->regval) && (pentry->regaddr != 136)) {
	  xil_printf("Verify Mismatch:  Reg=%d  Exp=0x%02x  Act=0x%02x\r\n", pentry->regaddr, pentry->regval, rdbuf[0]);
	  return;
	}
	pentry++;
  }
  xil_printf("...all registers verified!\r\n");
}

void check_SI5324C(void) {
  // checks read against programmed values
  u8 rdbuf[6];
  i2c_dev_init_entry_t* pentry = &SI5324C_init_record[0];

  xil_printf("Checking Programming for CLKC SI5324\r\n");
  while (pentry->regaddr != ENDOFTBL) {
	if (!read_i2c_reg(XPAR_CLOCK_IIC_BASEADDR, SI5324C_I2C_ADDR, (u8) pentry->regaddr, &rdbuf[0])) {
	  xil_printf("?Read Failure at I2C Device Address 0x%08x, reg address 0x%08x", SI5324A_I2C_ADDR, (u8) pentry->regaddr);
	  return;
	}
	if ((rdbuf[0] != pentry->regval) && (pentry->regaddr != 136)) {
	  xil_printf("Verify Mismatch:  Reg=%d  Exp=0x%02x  Act=0x%02x\r\n", pentry->regaddr, pentry->regval, rdbuf[0]);
	  return;
	}
	pentry++;
  }
  xil_printf("...all registers verified!\r\n");
}


void init_SI5324C(void) {
  i2c_dev_init_entry_t* pentry = &SI5324C_init_record[0];
  while (pentry->regaddr != ENDOFTBL) {
	if (!write_i2c_reg(XPAR_CLOCK_IIC_BASEADDR, SI5324C_I2C_ADDR, (u8) pentry->regaddr, pentry->regval)) {
	  xil_printf("?Write Failure at I2C Device Address 0x%08x, reg address 0x%08x", DS25CP104_I2C_ADDR, (u8) pentry->regaddr);
	  break;
	}
	pentry++;
  }
}


int write_i2c_reg(u32 i2c_driver, u8 i2c_addr, u8 regaddr, u8 wrdata) {
  // uses low-level drivers to write to a i2c register
  // returns 1 if write successful, 0 if it failed
  u8 wrbuf[4];
  unsigned int wrcount;

  wrbuf[0] = regaddr;
  wrbuf[1] = wrdata;

  wrcount = XIic_Send(i2c_driver, i2c_addr, &wrbuf[0], 2, XIIC_STOP);
  if (wrcount == 2)
	return 1;
  else {
	xil_printf("?I2C Send Error\r\n");
	return 0;
  }
}


int read_i2c_reg(u32 i2c_driver, u8 i2c_addr, u8 regaddr, u8* prdvalue) {
  // uses low-level driver to read from an i2c register
  // returns 1 if read successful, otherwise 0
  u8 wrbuf[4];

  unsigned int xcount;
  wrbuf[0] = regaddr;

  xcount = XIic_Send(i2c_driver, i2c_addr, &wrbuf[0], 1, XIIC_STOP);
  if (xcount != 1) {
	xil_printf("?I2C Send Error on Register Read\r\n");
	return 0;
  }
  xcount = XIic_Recv(i2c_driver, i2c_addr, &wrbuf[0], 1, XIIC_STOP);
  *prdvalue = wrbuf[0];
  if (xcount == 1)
	return 1;
  else {
	xil_printf("?I2C Recv Error on Register Read\r\n");
	return 0;
  }
}




