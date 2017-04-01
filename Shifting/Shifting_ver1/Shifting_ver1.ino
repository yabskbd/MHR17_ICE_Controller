// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>

#define BOOT_UP_ID 0x701
#define POSITION_MESSAGE 0x281

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


void wait_for_bootup_message(MCP_CAN CAN) {
  // check for bootup message
    int i = 0;
    INT8U len;
    INT8U buf[8];
    INT32U id;
    Serial.print("Waiting for bootup message:");
    while(CAN_MSGAVAIL != CAN.checkReceive())
    {
        Serial.print('.');
        i++;
        if(i > 60)
        {
            Serial.print('\n');
            i = 0;
        }
        delay(10);
    }
    CAN.readMsgBuf(&len, buf);
    id = CAN.getCanId();

    // check for bootup message
    if((id==BOOT_UP_ID)&&(buf[0]==0x00))
    {
        Serial.print("Boot up message recieved");
    }
    Serial.print('\n');

    return;
}

void setup()
{
    Serial.begin(115200);
    
    while(CAN_OK != CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }

    Serial.print("\n\n");
    Serial.println("CAN BUS Shield init ok!");

    INT8U len = 8;
    INT8U buf[8];
    INT32U id;
    
    // reset comm
    Serial.println("Reseting comm:");
    id = 0x000;
    len = 2;
    buf[0] = 0x82;
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);
    wait_for_bootup_message(CAN);
    
    /*// Entering Preop mode
    Serial.println("Entering preop mode");
    id = 0x000;
    len = 2;
    buf[0] = 0x80;
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);
    //delay(100);*/
    
    // start CAN open node
    Serial.println("Starting CAN node:");
    id = 0x000;
    len = 2;
    buf[0] = 0x01;
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);

    //delay(100);
    // SHUTDOWN
    Serial.println("Shutdown driver:");
    id = 0x201;
    len = 2;
    buf[0] = 0x06;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

    // SWITCHON
    Serial.println("SWITCHON driver:");
    id = 0x201;
    len = 2;
    buf[0] = 0x07;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

    // ENOP
    Serial.println("ENOP driver:");
    id = 0x201;
    len = 2;
    buf[0] = 0x0F;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

    /*// check for bootup message
    int i = 0;
    Serial.print("Waiting for bootup message:");
    while(CAN_MSGAVAIL != CAN.checkReceive())
    {
        Serial.print('.');
        i++;
        if(i > 60)
        {
            Serial.print('\n');
            i = 0;
        }
        delay(10);
    }
    CAN.readMsgBuf(&len, buf);
    id = CAN.getCanId();

    // check for bootup message
    if((id==BOOT_UP_ID)&&(buf[0]==0x00))
    {
        Serial.print("Boot up message recieved");
    }
    Serial.print('\n');*/

    /*// switch drive to "Ready to Switch on state"
    Serial.println("Enter Ready to Switch on state:");
    id = 0x201;
    len = 2;
    buf[0] = 0x06;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

    // switch drive to "Switched On State
    Serial.println("Enter Switched On State:");
    id = 0x201;
    len = 2;
    buf[0] = 0x07;
    buf[1] = 0x06;
    CAN.sendMsgBuf(id, 0, len, buf);

    // enter operation Enabled state
    Serial.println("Enter Operation Enabled mode:");
    id = 0x201;
    len = 2;
    buf[0] = 0x0F;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);*/

    /*// set mode of operation
    id = 0x201;
    len = 5;
    buf[0] = 0x60;
    buf[1] = 0x60;
    buf[2] = 0x00;
    buf[3] = 0x01;
    buf[4] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(1000);*/

    /*// check mode of operation
     * //consider using a 1 in RTR
    id = 0x181;
    len = 2;
    buf[0] = 0x60;
    buf[1] = 0x60;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(1000);*/

    /*//enter enable operation command
    Serial.println("Enable Controller Operation:");
    len = 2;
    buf[0] = 0x0F;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);*/

/*
    // enter new setpoint of 240000
    len = 6;
    id = 0x301;
    buf[0] = 0x00; 
    buf[1] = 0x20;
    buf[2] = 0x80;
    buf[3] = 0xA9;
    buf[4] = 0x03;
    buf[4] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(100);*/

    /*//enter enable operation command
    id = 0x181;
    len = 2;
    buf[0] = 0x7A;
    buf[1] = 0x60;
    CAN.sendMsgBuf(id, 0, len, buf);*/

    // set to profile position mode
    delay(1000);
    id = 0x601;
    len = 8;
    buf[0] = 0x2F;
    buf[1] = 0x60;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x01;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

    // check status of mode
    delay(100);
    id = 0x601;
    len = 8;
    buf[0] = 0x40;
    buf[1] = 0x60;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);

   
    
}

void loop()
{   

    INT8U len = 8;
    INT8U buf1[8];
    INT32U id;
    
    while(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        Serial.print("Message Recieved:\t");
          
        CAN.readMsgBuf(&len, buf1);

        id = CAN.getCanId();

        // print out can message
        switch(id)
        {
            case BOOT_UP_ID:
            {
                if(buf1[0] == 0x00)
                {
                    Serial.print("Boot up message\n");
                } else {
                    Serial.print("ERROR: Bootup message does not return 0x00");
                }
                break;
            }
            /*case POSITION_MESSAGE:
            {
                    Serial.print("Position message\n");
                    break;
            }*/
            case 0x181:
            {
              if (buf1[0] == 0x21) {
                Serial.println("Moving!");
                 delay(100);
                // move incrementally 2400000
                id = 0x601;
                len = 8;
                buf1[0] = 0x23;
                buf1[1] = 0x7A;
                buf1[2] = 0x60;
                buf1[3] = 0x00;
                buf1[4] = 0x80;
                buf1[5] = 0xA9;
                buf1[6] = 0x03;
                buf1[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf1);
            
               /* id = 0x601;
                len = 8;
                buf[0] = 0x2B
                buf[1] = 0x83;
                buf[2] = 0x60;
                buf[3] = 0x00;
                buf[4] = 0xE8;
                buf[5] = 0x03;
                buf[6] = 0x00;
                buf[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf);
            
                id = 0x601;
                len = 8;
                buf[0] = 0x2B
                buf[1] = 0x84;
                buf[2] = 0x60;
                buf[3] = 0x00;
                buf[4] = 0xE8;
                buf[5] = 0x03;
                buf[6] = 0x00;
                buf[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf);
            
                id = 0x601;
                len = 8;
                buf[0] = 0x2F
                buf[1] = 0x86;
                buf[2] = 0x60;
                buf[3] = 0x00;
                buf[4] = 0x01;
                buf[5] = 0x00;
                buf[6] = 0x00;
                buf[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf); */
            
                
                // Set the control word to "on"
                id = 0x601;
                len = 8;
                buf1[0] = 0x2B;
                buf1[1] = 0x40;
                buf1[2] = 0x60;
                buf1[3] = 0x00;
                buf1[4] = 0x5F;
                buf1[5] = 0x00;
                buf1[6] = 0x00;
                buf1[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf1);

                // Set the control word to "off"
                id = 0x601;
                len = 8;
                buf1[0] = 0x2B;
                buf1[1] = 0x40;
                buf1[2] = 0x60;
                buf1[3] = 0x00;
                buf1[4] = 0x0F;
                buf1[5] = 0x00;
                buf1[6] = 0x00;
                buf1[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf1);
                
                id = 0x601;
                len = 8;
                buf1[0] = 0x23;
                buf1[1] = 0x7A;
                buf1[2] = 0x60;
                buf1[3] = 0x00;
                buf1[4] = 0x80;
                buf1[5] = 0x56;
                buf1[6] = 0xFC;
                buf1[7] = 0xFF;
                CAN.sendMsgBuf(id, 0, len, buf1);

                // Set the control word to "on"
                id = 0x601;
                len = 8;
                buf1[0] = 0x2B;
                buf1[1] = 0x40;
                buf1[2] = 0x60;
                buf1[3] = 0x00;
                buf1[4] = 0x5F;
                buf1[5] = 0x00;
                buf1[6] = 0x00;
                buf1[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf1);
            
                /*// Set the control word to "off"
                id = 0x601;
                len = 8;
                buf[0] = 0x2B;
                buf[1] = 0x40;
                buf[2] = 0x60;
                buf[3] = 0x00;
                buf[4] = 0x0F;
                buf[5] = 0x00;
                buf[6] = 0x00;
                buf[7] = 0x00;
                CAN.sendMsgBuf(id, 0, len, buf);*/
              }
              break;
        }
            default:
            {
                Serial.print("CAN ID: ");
                Serial.print(id, HEX);
                Serial.print('\t');
                Serial.print("Message: ");
                for(int i = 0; i < len; i++)
                {
                    Serial.print(buf1[i],HEX);
                    Serial.print('\t');
                }
                Serial.print("\n");
                break;
            }
        }
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
