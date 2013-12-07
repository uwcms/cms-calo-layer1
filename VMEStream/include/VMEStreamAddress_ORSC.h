/*
 * Central definition of VME addresses used by VMEStream
 *
 *            VME_ADDR  VME_DIR MB_ADDR     MB_DIR
 *      REG1    0x2000  R/W     0x10008F00  R/W
 *      REG2    0x2002  R/W     0x10008F04  R/W
 *      REG3    0x2004  R/W     0x10008F08  R/W
 *      REG4    0x2006  R/W     0x10008F0C  R/W
 *      RAM1    0x0     WRITE   0x10000000  READ
 *      RAM2    0x800   WRITE   0x10001000  READ
 *      RAM3    0x1000  READ    0x10002000  WRITE
 *      RAM4    0x1800  READ    0x10003000  WRITE
 */

#define PC_2_ORSC_SIZE 0x10008F00
#define ORSC_2_PC_SIZE 0x10008F04

#define PC_2_ORSC_DATA 0x10000000
#define ORSC_2_PC_DATA 0x10002000

#define VMERAMSIZE 256
