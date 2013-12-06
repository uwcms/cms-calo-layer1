#include "xio.h"
#include "flashled.h"

typedef struct {
  int regaddr;
  u32 regval;
} ledflash;

#define ENDOFTBL      (-1)

ledflash green_tbl[]={
  {0x10008000, 0}, // - frontpanel LED1 green
  {0x10008004, 1}, //- frontpanel LED1 red
  {0x10008008, 1}, // - frontpanel LED2 green
  {0x1000800C, 0},// - frontpanel LED2 red
  {0x10008010, 0}, // - frontpanel LED3 green
  {0x10008014, 1}, // - frontpanel LED3 red
  {ENDOFTBL, 0}
};

ledflash red_tbl[]={
  {0x10008000, 1}, // - frontpanel LED1 green
  {0x10008004, 0}, //- frontpanel LED1 red
  {0x10008008, 0}, // - frontpanel LED2 green
  {0x1000800C, 1},// - frontpanel LED2 red
  {0x10008010, 1}, // - frontpanel LED3 green
  {0x10008014, 0}, // - frontpanel LED3 red
  {ENDOFTBL, 0}
};


void red_turnon(void){
  ledflash *fl = &red_tbl[0];
  while(fl->regaddr != ENDOFTBL){
    XIo_Out32(fl->regaddr, fl->regval);
    fl++;
  }
}

void green_turnon(void){
  ledflash *fl = &green_tbl[0];
  while(fl->regaddr != ENDOFTBL){
    XIo_Out32(fl->regaddr, fl->regval);
    fl++;
  }
}
