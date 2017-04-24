// can_open_lib.h
//
// Created by: Andrew Stenberg
// Date: April 24, 2017
#ifndef CAN_OPEN_LIB_H
#define CAN_OPEN_LIB_H

// libraries for CAN
#include <mcp_can.h>
#include <mcp_can_dfs.h>

// check that data types are defined
#typedef INT8U  unsigned char
#typedef INT8S  signed char
#typedef INT16U unsigned int
#typedef INT16S signed int
#typedef INT32U unsigned long
#typedef INT32S signed long

// CAN IDs for specifying objects
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

void recieve message(MCP_CAN CAN)
{
    
}

#endif
