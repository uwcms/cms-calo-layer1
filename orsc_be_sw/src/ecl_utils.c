#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "ecl_config.h"


#define SHIFT_REG_DATA_ADDR 0x10008024
#define MUX_REG_CLK_ADDR    0x10008028
#define DELAY_REG_CLK_ADDR  0x1000802C
#define SHIFT_REG_RST_ADDR  0x10008030
#define STB_DATA_ADDR       0x1000803C
#define AVAGO_CLK_RST_ADDR  0x10008040
#define CLK_A_RST_ADDR      0x10008044
#define CLK_C_RST_ADDR      0x10008048

//static u16 ecl_value;
static u8 mux_state;

//static u8 delay_state;

void bang_bits(u8 toggle, u32 address)
{
  if(toggle)
    XIo_Out32(address, 1);
  else
    XIo_Out32(address, 0);
}

void ecl_set_muxreg_raw_z(u8 value)
{
  int index;
  for(index = 7; index >= 0; --index)
  {
    bang_bits((value & (1 << index)), SHIFT_REG_DATA_ADDR);
    bang_bits(0, MUX_REG_CLK_ADDR);
    bang_bits(1, MUX_REG_CLK_ADDR);
  }
}

void ecl_set_delay_raw_z(u8 value, int stb_index)
{
  int index;
  u8 delay_state = 0xFF;
  for(index = 7; index >= 0; --index)
  {
    bang_bits((value & (1 << index)), SHIFT_REG_DATA_ADDR);
    bang_bits(0, DELAY_REG_CLK_ADDR);
    bang_bits(1, DELAY_REG_CLK_ADDR);
  }

  delay_state &= ~(0x01 << stb_index);

  bang_bits(delay_state, STB_DATA_ADDR);
  bang_bits(0xFF, STB_DATA_ADDR);
}

void ecl_set_delay_z(u8 value, int index)
{
  if(index < 0 || index > 7) return;

//  delay_state[index] = value;
  ecl_set_delay_raw_z(value, index);
}

void ecl_set_mux_fields_z(mux_reg_field_t field, u8 value)
{
  int field_base;

  switch (field)
  {
  case CLK:
    field_base = 0;
    break;
  case RST:
    field_base = 2;
    break;
  case BX0:
    field_base = 4;
    break;
  case L1A:
    field_base = 6;
    break;
  default:
    return;
  }

  ecl_set_muxreg_raw_z((value & 0x03) << field_base);
}

void ecl_config_init_z()
{
 // ecl_value = 0xff;
  XIo_Out32(DELAY_REG_CLK_ADDR, 0x01);
  XIo_Out32(MUX_REG_CLK_ADDR, 0x01);
  XIo_Out32(SHIFT_REG_RST_ADDR, 0x01);
  XIo_Out32(SHIFT_REG_RST_ADDR, 0x00);

  mux_state = 0;
  ecl_set_muxreg_raw_z(mux_state);
}
