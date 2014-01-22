#define REG1_ADDR      0x10008F00
#define REG2_ADDR      0x10008F04
#define REG3_ADDR      0x10008F08
#define REG4_ADDR      0x10008F0C

#define RAM1_ADDR      0x10000000  /*(MB-READ)*/
#define RAM2_ADDR      0x10001000  /*(MB-READ)*/
#define RAM3_ADDR      0x10002000  /*(MB-WRITE)*/
#define RAM4_ADDR      0x10003000  /*(MB-WRITE)*/

/* Configure the clock and set this register to 1*/
/* FE will read this register value before configure */
#define CLOCK_READY    0x10008050 
