// can_open_lib.h
//
// Created by: Andrew Stenberg
// Date: April 24, 2017

// check that data types are defined
#ifndef INT8U
#define INT8U  unsigned char
#endif

#ifndef INT8S
#define INT8S signed char
#endif

#ifndef INT16U
#define INT16U unsigned int
#endif

#ifndef INT16S
#define INT16S signed int
#endif

#ifndef INT32U
#define INT32U unsigned long
#endif

#ifndef INT32S
#define INT32S signed long
#endif

#ifndef CAN_OPEN_LIB_H
#define CAN_OPEN_LIB_H

// libraries for CAN
#include <mcp_can.h>
#include <mcp_can_dfs.h>


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

// returns once CAN message is recieved
void wait_for_message(MCP_CAN* CAN)
{
    int i = 0;
    while(CAN_MSGAVAIL != CAN->checkReceive())
    {
        Serial.print('.');
        i++;
        if(i > 60)
        {
            Serial.print('/n');
            i = 0;
        }
        delay(10);
    }
    return;
}

struct PDO_Receive
{
    INT16U statusword;
    INT32U position;
};

// checks that the proper PDO message is received, takes in PDO_Receive
// object, changes object values. returns 1 if proper message is received
// otherwise returns 0;
int wait_recieve_PDO(MCP_CAN* CAN, INT32U CAN_ID, PDO_Receive* data)
{
    INT8U buf[8];
    INT32U id;
    INT8U len;

    // wait for message to become available
    Serial.print("Waiting for PDO Message");
    wait_for_message(CAN);
 
    // Read in message
    CAN->readMsgBuf(&len, buf);
    id = CAN->getCanId();

    // check for proper id
    if(id != CAN_ID)
        return 0;

    switch(id)
    {
        case PDO1_TX_ID:
            data->statusword = (buf[1]<<8) + buf[0];
            return 1;
            break;
        case PDO2_TX_ID:
            data->statusword = (buf[1]<<8) + buf[0];
            data->position = (buf[5]<<24)+(buf[4]<<16)+(buf[3]<<8)+buf[2];
            return 1;
            break;
        default:
            return 0; // did not recognize message
            break;
    }
}

#endif
