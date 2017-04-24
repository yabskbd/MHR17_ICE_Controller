#ifndef shifting_setup_H
#define shifting_setup_H

#define BOOT_UP_ID 0x701
#define POSITION_MESSAGE 0x281
#define CHECK_ENABLED_ID 0x281
#define DSPACE_SHIFT_UP 0x10
#define DSPACE_SHIFT_DOWN 0x05
#define DSPACE_SHIFT_HALF 0x06


int wait_for_bootup_message(MCP_CAN CAN);
int check_if_enabled(MCP_CAN CAN);
int check_for_ack(MCP_CAN CAN,int Mode);
int homing_mode(MCP_CAN CAN);

long START_POS = 0;
long CURR_POS = 0;
INT8U int_pos_time = 1;

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
  
  //////////GETTING READY TO MOVE BACK  
  delay(1000); 
  
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
         
      Serial.print("CURR_POS: ");
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
  
  long diff = START_POS - CURR_POS;
  Serial.print("Diff:");
  Serial.println(diff);
  
  //Moving BACK
  id = 0x601;
  len = 8;
  buf1[0] = 0x23;
  buf1[1] =  0x7A; 
  buf1[2] = 0x60;
  buf1[3] = 0x00;
  buf1[4] = diff ;//LSB
  buf1[5] = (diff>>8);
  buf1[6] = (diff>>8);
  buf1[7] =  (diff>>8);//MSB
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



int init_shifting(MCP_CAN CAN,int Mode){
    INT8U len = 8;
    INT8U buf[8];
    INT8U i = 0;
    INT32U id;
    
    // reset comm
    Serial.println("Reseting comm:");
    id = 0x000;
    len = 2;
    buf[0] = 0x82; //82 also possible
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);
    if (!wait_for_bootup_message(CAN)) {
      Serial.println("Failled Boot!"); 
      return 0;
    }
    
    
    // start CAN open node
    Serial.println("Starting CAN node:");
    id = 0x000;
    len = 2;
    buf[0] = 0x01;
    buf[1] = 0x01;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(100);
    
    // SHUTDOWN
    Serial.println("Shutdown driver:");
    id = 0x201;
    len = 2;
    buf[0] = 0x06;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(100);

    // SWITCHON
    Serial.println("SWITCHON driver:"); // not required
    id = 0x201;
    len = 2;
    buf[0] = 0x07;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    delay(100);

    // ENOP
    Serial.println("ENOP driver:");
    id = 0x201;
    len = 2;
    buf[0] = 0x0F;
    buf[1] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    if (!check_if_enabled(CAN)) {
      Serial.println("Failled ENABLED!"); 
      return 0;
    }

    delay(100);
    // set to profile Homming mode 0x06
    id = 0x601;
    len = 8;
    buf[0] = 0x2F;
    buf[1] = 0x60;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = Mode; //Sets Mode 0x01 for position 0x06 for Homing
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    if (!check_for_ack(CAN,Mode)) {
      Serial.println("Failled ACK!"); 
      return 0;
    }
    
    

//    // check status of mode
//    delay(100);
//    id = 0x601;
//    len = 8;
//    buf[0] = 0x40;
//    buf[1] = 0x60;
//    buf[2] = 0x60;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    buf[6] = 0x00;
//    buf[7] = 0x00;
//    CAN.sendMsgBuf(id, 0, len, buf);

    return 1;
}

int wait_for_bootup_message(MCP_CAN CAN) {
  // check for bootup message
    int i = 0;
    int wait = 0;
    
    INT8U buf[8];
    INT32U id;
    INT8U len;
    Serial.print("Waiting for bootup message:");
    while(CAN_MSGAVAIL != CAN.checkReceive() && wait++ < 100)
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

    // check for bootup message
    if((id==BOOT_UP_ID)&&(buf[0]==0x00))
    {
        Serial.print("Boot up message recieved");
        return 1;
    }
    Serial.print('\n');

    return 0;
}

int check_if_enabled(MCP_CAN CAN) {
    int i = 0;
    int wait = 0;
    
    INT8U buf[8];
    INT32U id;
    INT8U len;
    Serial.print("Checking if enabled");
    
    // request status message
    id = 0x601;
    len = 8;
    buf[0] = 0x40;
    buf[1] = 0x41;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    
    //sift through up to 4 messages
    int sift_through = 0;
    while(sift_through<4)
    {
        // wait for status message
        while(CAN_MSGAVAIL != CAN.checkReceive() && wait++ < 10)
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
        if(((id==0x581))&&(buf[1]==0x41)&&(buf[2]==0x60)&&(buf[3]==0x00))
        {
            // check if enabled
            if((buf[4]&(1<<2)&&(~buf[4]&(1<<3))&&(buf[4]&(1<<5))))
            {
                Serial.println("Device now in Operation Enabled State");
                return 1;
            }
        }
        
        if(((id==0x181)))
        {
            // check if enabled
            if((buf[0]&(1<<2)&&(~buf[0]&(1<<3))&&(buf[0]&(1<<5))))
            {
                Serial.println("Device now in Operation Enabled State");
                return 1;
            }
        }
        
        
    }
    Serial.print('\n');
    Serial.println("Failled ACK!"); 
    return 0;
}

int check_for_ack(MCP_CAN CAN, int Mode)
{
    int i = 0;
    int wait = 0;
    
    INT8U buf[8];
    INT32U id;
    INT8U len;
    Serial.print("Checking ACK:"); 
    
    // request status message
    id = 0x601;
    len = 8;
    buf[0] = 0x40;
    buf[1] = 0x61;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    //sift through up to 4 messages
    int sift_through = 0;
    while(sift_through<4)
    {
        // wait for status message
        while(CAN_MSGAVAIL != CAN.checkReceive() && wait++ < 100)
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
        
        if (wait >= 100) {
          return 0;
        }
        CAN.readMsgBuf(&len, buf);
        id = CAN.getCanId();
        Serial.print("CAN ID: ");
        Serial.print(id, HEX);
        Serial.print('\t');
        Serial.print("Message: ");
        for(int i = 0; i < len; i++)
        {
            Serial.print(buf[i],HEX);
            Serial.print('\t');
        }
       Serial.print("\n");
        // check for statusword
        if((id==0x581)&&(buf[1]==0x61)&&(buf[2]==0x60)&&(buf[3]==0x00)&&(buf[4]==Mode))
        {
           Serial.println("Got ACK for position mode"); 
           return 1;
        }
    }
    Serial.print('\n');

    return 0;
}

int check_homing(MCP_CAN CAN)
{
    int i = 0;
    int wait = 0;
    
    INT8U buf[8];
    INT32U id;
    INT8U len;
    Serial.print("Checking Homing: "); 
    
   // request status message
    id = 0x601;
    len = 8;
    buf[0] = 0x40;
    buf[1] = 0x41;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    CAN.sendMsgBuf(id, 0, len, buf);
    
    
    //sift through up to 4 messages
    int sift_through = 0;
    while(sift_through<4)
    {
        // wait for status message
        while(CAN_MSGAVAIL != CAN.checkReceive() && wait++ < 100)
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
        
        if (wait >= 100) {
          return 0;
        }
        CAN.readMsgBuf(&len, buf);
        id = CAN.getCanId();
        
        Serial.print("CAN ID: ");
        Serial.print(id, HEX);
        Serial.print('\t');
        Serial.print("Message: ");
        for(int i = 0; i < len; i++)
        {
            Serial.print(buf[i],HEX);
            Serial.print('\t');
        }
        Serial.print("\n");
    
        // check for statusword Homing
        if( (id==0x581) && (buf[1]==0x41) && (buf[2]==0x60) && (buf[3]==0) && (buf[5] & (1<<4) ) && (buf[5] & (1<<2) ) )
        {
           Serial.println("Homing Success"); 
           return 1;
        }
    }
    Serial.print('\n');

    return 0;
}


int homing_mode(MCP_CAN CAN){

    INT8U len = 8;
    INT8U buf1[8];
    INT32U id = 0;
//--------------------------------------------------------------//    
    //Homing Mode Method: Reset Reference position (download req)
    id = 0x601;
    len = 8;
    buf1[0] = 0x2F;
    buf1[1] = 0x98;
    buf1[2] = 0x60;
    buf1[3] = 0x00;
    buf1[4] = 0x25;
    buf1[5] = 0x00;
    buf1[6] = 0x00;
    buf1[7] = 0x00; 
    CAN.sendMsgBuf(id, 0, len, buf1);
    
    delay(100);
    //Set Homing Mode to rest reference position
    id = 0x601;
    len = 8;
    buf1[0] = 0x2B;
    buf1[1] = 0x40;
    buf1[2] = 0x60;
    buf1[3] = 0x00;
    buf1[4] = 0x1F; //F preserves mode
    buf1[5] = 0x00;
    buf1[6] = 0x00;
    buf1[7] = 0x00; 
    CAN.sendMsgBuf(id, 0, len, buf1);
    delay(100);
    if(!check_homing(CAN)){
      Serial.println("Homing Failled");
      return 0;
    }

    return 1;

}

#endif
