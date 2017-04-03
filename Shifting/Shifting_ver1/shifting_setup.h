#ifndef shifting_setup_H
#define shifting_setup_H

#define BOOT_UP_ID 0x701
#define POSITION_MESSAGE 0x281
#define CHECK_ENABLED_ID 0x281
#define DSPACE_SHIFT_UP 0x10
#define DSPACE_SHIFT_DOWN 0x05
#define DSPACE_SHIFT_HALF 0x06


int wait_for_bootup_message(MCP_CAN CAN);

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
    if (!wait_for_bootup_message(CAN)) {
      return 0;
    }
    
    
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
      Serial.println("Failled To initialize");
      delay(1000);      
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

int wait_for_bootup_message(MCP_CAN CAN) {
  // check for bootup message
    int i = 0;
    int wait = 0;
    
    INT8U buf[8];
    INT32U id;
    INT8U len;
    Serial.print("Waiting for bootup message:");
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

    // check for bootup message
    if((id==BOOT_UP_ID)&&(buf[0]==0x00))
    {
        Serial.print("Boot up message recieved");
        return 1;
    }
    Serial.print('\n');

    return 0;
}

#endif
