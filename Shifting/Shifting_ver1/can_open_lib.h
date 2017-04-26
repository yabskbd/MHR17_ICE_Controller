// can_open_lib.h
//
// Created by: Andrew Stenberg
// Date: April 24, 2017

#include "can_open_dfs.h"

#ifndef CAN_OPEN_LIB_H
#define CAN_OPEN_LIB_H

// libraries for CAN
#include <mcp_can.h>
#include <mcp_can_dfs.h>

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
    INT32S position;
};

// checks that the proper PDO message is received, takes in PDO_Receive
// object, changes object values. returns 1 if proper message is received
// otherwise returns 0;
int pdo_wait_recieve(MCP_CAN* CAN, INT32U CAN_ID, PDO_Receive* data)
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

// send a download request to motion controller
// data is pointer to an array that is data[0]->low byte, data[n]->higher byte
void sdo_download_request(MCP_CAN* CAN, INT16U index, INT8U subindex, INT8U CS, INT8U* data)
{
    INT8U buf[8];    // buffer for sending data
    buf[0] = CS;
    buf[1] = index & 0xFF; // ensure low byte
    buf[2] = index >> 8;   // high byte
    buf[3] = subindex;
    
    switch(CS) // load data based on command specifier
    {
        case DOWNLOAD_1_BYTES:
            buf[4] = *data;
            buf[5] = 0x00;
            buf[6] = 0x00;
            buf[7] = 0x00;
            break;
        case DOWNLOAD_2_BYTES:
            buf[4] = *data++;
            buf[5] = *data;
            buf[6] = 0x00;
            buf[7] = 0x00;
            break;
        case DOWNLOAD_3_BYTES:
            buf[4] = *data++;
            buf[5] = *data++;
            buf[6] = *data;
            buf[7] = 0x00;
            break;
        case DOWNLOAD_4_BYTES:
            buf[4] = *data++;
            buf[5] = *data++;
            buf[6] = *data++;
            buf[7] = *data;
            break;
        default:
            break;
    } // switch

    // send SDO packet
    CAN->sendMsgBuf(SDO_RX_ID, 0, 8, buf);
}

int sdo_wait_download_response(MCP_CAN* CAN, INT16U index, INT8U subindex)
{
    INT8U buf[8];
    INT8U len;
    INT32U id;
    
    // wait for and load in message
    wait_for_message(CAN);
    CAN->readMsgBuf(&len, buf);
    id = CAN->getCanId();
    
    // check can id
    if(id != SDO_TX_ID)
        return 0;
    // check command specifier
    if(buf[0] != 0x60)
        return 0;
    // check index
    else if(buf[1] != (index & 0xFF)) // lowbyte
        return 0;
    else if(buf[2] != (index >> 8)) // highbyte
        return 0;
    // check subindex
    else if(buf[3] != subindex)
        return 0;
    else
        return 1;
}

#endif











