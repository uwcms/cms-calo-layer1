/*
 * Central definition of VME addresses used by VMEStream
 *
 * MB BASEADDR = 0x10000000
 * VME BASEADDR for slot 0
 * 
 * MB and VME offsets from base address
 * --MB READS, VME WRITES
 * --Address Range: 0x0000-0x07FF
 * 
 * --MB READS, VME WRITES
 * --Address Range: 0x0800-0x0FFF
 * 
 * --VME READS, MB WRITES
 * --Address Range: 0x1000-0x17FF
 * 
 * --VME READS, MB WRITES
 * --Address Range: 0x1800-0x1FFF
 */

/*
#define ORSC_2_PC_SIZE 0x0800
#define PC_2_ORSC_SIZE 0x0000

#define ORSC_2_PC_DATA 0x0804
#define PC_2_ORSC_DATA 0x0004

#define VMERAMSIZE 511 // (0x800 - 0x004)/4
*/

#define PC_RECV_SIZE    0x10000000
#define PC_SEND_SIZE    0x10000004
#define PC2ORSC_DATA    0x10000008

#define ORSC_RECV_SIZE  0x10001000
#define ORSC_SEND_SIZE  0x10001004
#define ORSC2PC_DATA    0x10001008

#define VMERAMSIZE 510
