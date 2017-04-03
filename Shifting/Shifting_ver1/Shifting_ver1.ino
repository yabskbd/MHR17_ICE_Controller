// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include "shifting_setup.h"


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

int init_shifting(MCP_CAN CAN);

INT8U UPSHIFT[8];
INT8U DOWNSHIFT[8];
INT8U HALFSHIFT[8];

INT8U UPSHIFT_Back[8];
INT8U DOWNSHIFT_Back[8];
INT8U HALFSHIFT_Back[8];

void shift_pos(MCP_CAN CAN, INT8U * pos){

  INT8U buf1[8];
  INT32U id;
  INT8U len;
  
  //Moving 
  id = 0x601;
  len = 8;
  CAN.sendMsgBuf(id, 0, len, pos);
  
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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    while(CAN_OK != CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }

    Serial.print("\n\n");
    Serial.println("CAN BUS Shield init ok!");
     
    while(!init_shifting(CAN)){
        //Wating for shifting to be enabled
    }
    
    pinMode(5,INPUT);
    digitalWrite(5, INPUT_PULLUP);
    pinMode(7,INPUT);
    digitalWrite(7, INPUT_PULLUP);
    pinMode(3,INPUT);
    digitalWrite(3,INPUT_PULLUP);
    
    //UPSHIFT
    // move incrementally POSTIVE + 185
    UPSHIFT[0] = 0x23;
    UPSHIFT[1] = 0x7A;
    UPSHIFT[2] = 0x60;
    UPSHIFT[3] = 0x00;
    UPSHIFT[4] = 0xB9; //LSB
    UPSHIFT[5] = 0x00;
    UPSHIFT[6] = 0x00; 
    UPSHIFT[7] = 0x00; //MSB
    
    UPSHIFT_Back[0] = 0x23;
    UPSHIFT_Back[1] = 0x7A;
    UPSHIFT_Back[2] = 0x60;
    UPSHIFT_Back[3] = 0x00;
    UPSHIFT_Back[4] = 0x47; //LSB
    UPSHIFT_Back[5] = 0xFF;
    UPSHIFT_Back[6] = 0xFF; 
    UPSHIFT_Back[7] = 0xFF; //MSB
   
    
    //DOWNSHIFT
    //Moving back NEGATIVE - 210
    DOWNSHIFT[0] = 0x23;
    DOWNSHIFT[1] = 0x7A;
    DOWNSHIFT[2] = 0x60;
    DOWNSHIFT[3] = 0x00;
    DOWNSHIFT[4] = 0x2E; //LSB
    DOWNSHIFT[5] = 0xFF;
    DOWNSHIFT[6] = 0xFF; 
    DOWNSHIFT[7] = 0xFF; //MSB
    
    DOWNSHIFT_Back[0] = 0x23;
    DOWNSHIFT_Back[1] = 0x7A;
    DOWNSHIFT_Back[2] = 0x60;
    DOWNSHIFT_Back[3] = 0x00;
    DOWNSHIFT_Back[4] = 0xD2; //LSB
    DOWNSHIFT_Back[5] = 0x00;
    DOWNSHIFT_Back[6] = 0x00; 
    DOWNSHIFT_Back[7] = 0x00; //MSB
  
    //HALFSHIFT
    //Moving Postive 140
    HALFSHIFT[0] = 0x23;
    HALFSHIFT[1] = 0x7A;
    HALFSHIFT[2] = 0x60;
    HALFSHIFT[3] = 0x00;
    HALFSHIFT[4] = 0x8C; //LSB
    HALFSHIFT[5] = 0x00;
    HALFSHIFT[6] = 0x00;
    HALFSHIFT[7] = 0x00; //MSB
    
    HALFSHIFT_Back[0] = 0x23;
    HALFSHIFT_Back[1] = 0x7A;
    HALFSHIFT_Back[2] = 0x60;
    HALFSHIFT_Back[3] = 0x00;
    HALFSHIFT_Back[4] = 0x74; //LSB
    HALFSHIFT_Back[5] = 0xFF;
    HALFSHIFT_Back[6] = 0xFF;
    HALFSHIFT_Back[7] = 0xFF; //MSB
    
       
}

void loop()
{   

     
    //Ready to recieve commands to shift from dSPACE
    //Serial.println("Ready to recieve shfting commands");
    INT8U len = 8;
    INT8U buf1[8];
    INT32U id = 0;
    
    delay(180);
    if(digitalRead(5) && ~(digitalRead(7))){
      //SHIFT_UP
      id = DSPACE_SHIFT_DOWN;
    }
    
    if(digitalRead(7) && ~(digitalRead(5))){
      //SHIFT_UP
      id = DSPACE_SHIFT_UP;
    }
    
    if(digitalRead(3) == LOW){
      id = DSPACE_SHIFT_HALF;
    }
    //Serial.print("6:");
    //Serial.println(digitalRead(6));
    
    //Serial.print("A3:");
    //Serial.println(digitalRead(A3));
    
    //delay(5000);
    //while(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
         
        //CAN.readMsgBuf(&len, buf1);

        //id = CAN.getCanId();
        
        // print out can message

        switch(id)
        {
            case DSPACE_SHIFT_UP:
            {
              Serial.println("UPSHIFT!");
                shift_pos(CAN,UPSHIFT);
                delay(1000);
                shift_pos(CAN,UPSHIFT_Back);
                
                break;
                 
            }
            case DSPACE_SHIFT_DOWN:
            {
             Serial.println("DOWNSHIFT!");
                shift_pos(CAN,DOWNSHIFT);
                delay(1000);
                shift_pos(CAN,DOWNSHIFT_Back);
                break;
             }
            
            
            case DSPACE_SHIFT_HALF:
            {
              Serial.println("HALFSHIFT!");
                shift_pos(CAN,HALFSHIFT);  
                delay(1000);
                shift_pos(CAN,HALFSHIFT_Back);
                break;
            }
            
            default:
            {
//                Serial.print("CAN ID: ");
//                Serial.print(id, HEX);
//                Serial.print('\t');
//                Serial.print("Message: ");
//                for(int i = 0; i < len; i++)
//                {
//                    Serial.print(buf1[i],HEX);
//                    Serial.print('\t');
//                }
//                Serial.print("\n");
//                break;
            }
        }
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
