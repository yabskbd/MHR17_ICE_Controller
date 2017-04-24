// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include "shifting_setup.h"


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

int init_shifting(MCP_CAN CAN,int Mode);

INT8U UPSHIFT[8];
INT8U DOWNSHIFT[8];
INT8U HALFSHIFT[8];

INT8U UPSHIFT_Back[8];
INT8U DOWNSHIFT_Back[8];
INT8U HALFSHIFT_Back[8];


int check_sdo_rx(MCP_CAN CAN,int idx_low, int idx_high, int sub_idx){
//download response

  int i = 0;
  int wait = 0;
  
  INT8U buf[8];
  INT32U id;
  INT8U len;
    
    //sift through up to 4 messages
    int sift_through = 0;
    while(sift_through<4)
    {
        // wait for status message
        while(CAN_MSGAVAIL != CAN.checkReceive() && wait++ < 10000)
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
        
        if (wait >= 10000) {
          return 0;
        }
        CAN.readMsgBuf(&len, buf);
        id = CAN.getCanId();
    
        // check for statusword
        if((id==0x581)&&(buf[0]==0x60)&&(buf[1]==idx_low)&&(buf[2]==idx_high)&&(buf[3]==sub_idx))
        {
           Serial.println("Got Download Response!"); 
           return 1;
        }
    }
    Serial.print('\n');

    return 0;


}

void shift_pos_old(MCP_CAN CAN, INT8U * pos){

  INT8U buf1[8];
  INT32U id;
  INT8U len;
  
  //Moving 
  id = 0x601;
  len = 8;
  CAN.sendMsgBuf(id, 0, len, pos);
  
  if(!check_sdo_rx(CAN,0x7A,0x60,0x00)){
    Serial.println("Failled Position Download Response!");
  }
  
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
  
  if(!check_sdo_rx(CAN,0x40,0x60,0x00)){
    Serial.println("Failled Controlword ON Download Response!");
  }
  
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

  if(!check_sdo_rx(CAN,0x40,0x60,0x00)){
    Serial.println("Failled Controlword OFF Download Response!");
  }


}

void setup()
{
    Serial.begin(115200);
    delay(100);
    
    int int_counter = 0;
    while(int_counter < 1){ 
      
      while(CAN_OK != CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
      {
          Serial.println("CAN BUS Shield init fail");
          Serial.println("Init CAN BUS Shield again");
          delay(100);
      }
  
      Serial.print("\n\n");
      Serial.println("CAN BUS Shield init ok!");
      
      delay(1000);
      //Init with Homing
      while(!init_shifting(CAN,0x06)){
          delay(100);//Wating for shifting to be enabled
      }
      int_counter = int_counter + 1;
    }
    
    INT8U len = 8;
    INT8U buf1[8];
    INT32U id = 0;
//--------------------------------------------------------------//    
    //Homing Mode Reset Reference position
   while(!homing_mode(CAN)){
     Serial.println("Homing Mode Function Returned 0");
   
   }  
    Serial.println("Homing Success Moving To Profile Mode");
    
//    do{
//      // set to profile Profile Position Mode
//      id = 0x601;
//      len = 8;
//      buf1[0] = 0x2F;
//      buf1[1] = 0x60;
//      buf1[2] = 0x60;
//      buf1[3] = 0x00;
//      buf1[4] = 0x01; //Sets Mode 0x01 for position
//      buf1[5] = 0x00;
//      buf1[6] = 0x00;
//      buf1[7] = 0x00;
//      CAN.sendMsgBuf(id, 0, len, buf1);
//      
//    }while (!check_for_ack(CAN,0x01));
    
    delay(1000);
    //Init with Shifting Enabled
    int_counter = 0;
    while(int_counter < 4){
      while(!init_shifting(CAN,0x01)){
          delay(1000);//Wating for shifting to be enabled
      }
      delay(1000);
      ++int_counter;
    }
    
    pinMode(5,INPUT);
    digitalWrite(5, INPUT_PULLUP);
    pinMode(7,INPUT);
    digitalWrite(7, INPUT_PULLUP);
    pinMode(3,INPUT);
    digitalWrite(3,INPUT_PULLUP);
    //-----------------------------------//
    
    delay(100);   
    // Request Initial Position
    id = 0x601;
    len = 8;
    buf1[0] = 0x40;
    buf1[1] = 0x64;
    buf1[2] = 0x60;
    buf1[3] = 0x00;
    buf1[4] = 0x00;
    buf1[5] = 0x00;
    buf1[6] = 0x00;
    buf1[7] = 0x00; 
    CAN.sendMsgBuf(id, 0, len, buf1);
    
    Serial.println("Request Init Pos");
    delay(50);
    while(CAN_MSGAVAIL == CAN.checkReceive()){
      CAN.readMsgBuf(&len, buf1);

      id = CAN.getCanId();
      if(id==0x581 && buf1[1] == 0x64 && buf1[2]==0x60) {
        CURR_POS = buf1[7];
        CURR_POS = (CURR_POS << 8);
        CURR_POS = CURR_POS + buf1[6];
        CURR_POS = (CURR_POS << 8);
        CURR_POS = CURR_POS +  buf1[5];
        CURR_POS = (CURR_POS << 8);
        CURR_POS = CURR_POS + buf1[4];
        
        if(int_pos_time){
          START_POS = CURR_POS;
          int_pos_time = 0;
        }
        
        Serial.print("Inital_POS: ");
        Serial.println(CURR_POS);
        
      }

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
    }
    
    //UPSHIFT
    // move incrementally POSTIVE + 170
    UPSHIFT[0] = 0x23;
    UPSHIFT[1] = 0x7A;
    UPSHIFT[2] = 0x60;
    UPSHIFT[3] = 0x00;
    UPSHIFT[4] = 0xAA; //LSB
    UPSHIFT[5] = 0x00;
    UPSHIFT[6] = 0x00; 
    UPSHIFT[7] = 0x00; //MSB
    
    UPSHIFT_Back[0] = 0x23;
    UPSHIFT_Back[1] = 0x7A;
    UPSHIFT_Back[2] = 0x60;
    UPSHIFT_Back[3] = 0x00;
    UPSHIFT_Back[4] = 0x74; //LSB
    UPSHIFT_Back[5] = 0xFF;
    UPSHIFT_Back[6] = 0xFF; 
    UPSHIFT_Back[7] = 0xFF; //MSB
   
    
    //DOWNSHIFT
    //Moving back NEGATIVE - 170
    DOWNSHIFT[0] = 0x23;
    DOWNSHIFT[1] = 0x7A;
    DOWNSHIFT[2] = 0x60;
    DOWNSHIFT[3] = 0x00;
    DOWNSHIFT[4] = 0x56; //LSB
    DOWNSHIFT[5] = 0xFF;
    DOWNSHIFT[6] = 0xFF; 
    DOWNSHIFT[7] = 0xFF; //MSB
    
    DOWNSHIFT_Back[0] = 0x23;
    DOWNSHIFT_Back[1] = 0x7A;
    DOWNSHIFT_Back[2] = 0x60;
    DOWNSHIFT_Back[3] = 0x00;
    DOWNSHIFT_Back[4] = 0x5A; //LSB
    DOWNSHIFT_Back[5] = 0x00;
    DOWNSHIFT_Back[6] = 0x00; 
    DOWNSHIFT_Back[7] = 0x00; //MSB
  
    //HALFSHIFT
    //Moving Postive 80
    HALFSHIFT[0] = 0x23;
    HALFSHIFT[1] = 0x7A;
    HALFSHIFT[2] = 0x60;
    HALFSHIFT[3] = 0x00;
    HALFSHIFT[4] = 0x50; //LSB
    HALFSHIFT[5] = 0x00;
    HALFSHIFT[6] = 0x00;
    HALFSHIFT[7] = 0x00; //MSB
    
    HALFSHIFT_Back[0] = 0x23;
    HALFSHIFT_Back[1] = 0x7A;
    HALFSHIFT_Back[2] = 0x60;
    HALFSHIFT_Back[3] = 0x00;
    HALFSHIFT_Back[4] = 0x92; //LSB
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
         
//        CAN.readMsgBuf(&len, buf1);
//
//        id = CAN.getCanId();
        
        // print out can message

        switch(id)
        {
            case DSPACE_SHIFT_UP:
            {
              Serial.println("UPSHIFT!");
                shift_pos(CAN,UPSHIFT);
                delay(1000);
                check_for_ack(CAN,0x01);
                //shift_pos(CAN,UPSHIFT_Back);
                
                break;
                 
            }
            case DSPACE_SHIFT_DOWN:
            {
             Serial.println("DOWNSHIFT!");
                shift_pos(CAN,DOWNSHIFT);
                delay(1000);
                check_for_ack(CAN,0x01);
                //shift_pos(CAN,DOWNSHIFT_Back);
                break;
             }
            
            
            case DSPACE_SHIFT_HALF:
            {
              Serial.println("HALFSHIFT!");
                shift_pos(CAN,HALFSHIFT);  
                delay(1000);
                check_for_ack(CAN,0x01);
                //shift_pos(CAN,HALFSHIFT_Back);
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
