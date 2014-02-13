#define QPLL_LOCK_STATUS         0x100F0000        /* Read-Only 6-bits */
#define QPLL_RESET               0x100F0004        /* R/W 6-bits */
#define TX_RESET                 0x100F0008        /* R/W 23-bits */
#define TX_RESET_DONE            0x100F0010        /* R 23-bits */
#define ORBIT_LENGTH             0x100F0014        /* R/W 12-bits */
#define TX_USER_READY            0x100F0018        /* R/W 23-bits */
#define START_ALIGN_REG          0x100F001c        /* R/W 1-bit */
#define BX0_TEST_STATUS          0x100F0020        /* Ignore */
#define BC0_ALIGN_TEST           0x100F0024        /* R/W 1-bit Ignore */
#define FREE_RUN_BC0             0x100F0028        /* Ignore */
#define PATTERN_BX_LENGTH        0x100F002c        /* R/W 8-bits */
#define PRBS_GEN                 0x100F0030        /* R/W 23-bits */
#define RCT_CAPTURE              0x100F0034        /* R/W 1-bit */
#define CLOCK_CHIP_READY         0x100F0038        /* R 1-bit */
