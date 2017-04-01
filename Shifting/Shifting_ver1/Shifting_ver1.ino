// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>

#define BOOT_UP_ID 0x701
#define POSITION_MESSAGE 0x281
#define CHECK_ENABLED_ID 0x181
#define DSPACE_SHIFT_UP 0x10
#define DSPACE_SHIFT_DOWN 0x05
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin
void wait_for_bootup_message(MCP_CAN CAN);

int init_shifting(MCP_CAN CAN){
    INT8U len = 8;
    INT8U buf[8];
    INT8U i = 0;
    INT32U id;
    
    // reset comm
    Serial.println("Reseting comm:");
    id = 0x000;
    len = 2;
    buf[0] = 0x82;
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);
    wait_for_bootup_message(CAN);
    
    
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

    //Check If enabled
    Serial.println("Check if Enabled");
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

    if((id==CHECK_ENABLED_ID)&&(buf[0] != 0x21)){
      return 0; //RETURN and re do this whole thing
    }
    Serial.println("Shifting Enabled");

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

    return 1;
}

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
       
}

void loop()
{   

    
    INT8U shifting_ready = 0;

    while(!init_shifting){
        //Wating for shifting to be enabled
    }
    
    
    //Ready to recieve commands to shift from dSPACE
    Serial.println("Ready to recieve shfting commands");
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
                //Something Good Happened
              }
              break;
            }
            case DSPACE_SHIFT_UP:
            {
            
                Serial.println("Moving!");
                 delay(100);
                // move incrementally POSTIVE + 2400000
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
            }
            case DSPACE_SHIFT_DOWN:
            {
                
                //Moving back NEGATIVE - 2400000
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
