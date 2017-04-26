// can_open_dfs.h

// define char types
#ifndef INT8U
#define INT8U   unsigned char
#endif

#ifndef INT16U
#define INT16U  unsigned int
#endif

#ifndef INT32U
#define INT32U  unsigned long
#endif

#ifndef INT8S
#define INT8S   signed char
#endif

#ifndef INT16S
#define INT16S  signed int
#endif

#ifndef INT32S
#define INT32S  signed long
#endif

// CAN IDs for CAN Open objects
#define NMT_ID          0x000
#define SYNC_ID         0x080
#define EMERGENCY_ID    0X081
#define PDO1_TX_ID      0x181
#define PDO1_RX_ID      0x201
#define PDO2_TX_ID      0x281
#define PDO2_RX_ID      0x301
#define PDO3_TX_ID      0x381
#define PDO3_RX_ID      0x401
#define PDO4_TX_ID      0x481
#define PDO4_RX_ID      0x501
#define SDO_TX_ID       0x581
#define SDO_RX_ID       0x601
#define NMT_ERROR_ID    0x701

// command specifiers for SDO messages
#define UPLOAD_1_BYTES          0x4F
#define UPLOAD_2_BYTES          0x4B
#define UPLOAD_3_BYTES          0x47
#define UPLOAD_4_BYTES          0x43
#define DOWNLOAD_1_BYTES        0x2F
#define DOWNLOAD_2_BYTES        0x2B
#define DOWNLOAD_3_BYTES        0x27
#define DOWNLOAD_4_BYTES        0x23
#define DOWNLOAD_DONT_SPECIFY   0x22

// useful index and subindexes
#define CONTROLWORD_INDEX           0x6040
#define CONTROLWORD_SUBINDEX        0x00

#define STATUSWORD_INDEX            0x6041
#define STATUSWORD_SUBINDEX         0x00

#define OP_MODE_INDEX               0x6060
#define OP_MODE_SUBINDEX            0x00

#define DISPLAY_OP_MODE_INDEX       0x6061
#define DISPLAY_OP_MODE_SUBINDEX    0x00

#define POS_DEMAND_VALUE_INDEX      0x6062  // what the controller is demanding
#define POS_DEMAND_VALUE_SUBINDEX   0x00

#define POS_ACTUAL_VAL_INDEX        0x6064  // where motor actually is
#define POS_ACTUAL_VAL_SUBINDEX     0x00

#define TARGET_POSITION_INDEX       0x607A  // value to command new position
#define TARGET_POSITION_SUBINDEX    0x00

#define POSITION_RANGE_INDEX        0x607B
#define MIN_LIMIT_SUBINDEX          0x01
#define MAX_LIMIT_SUBINDEX          0x02

