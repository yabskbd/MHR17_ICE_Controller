/************************************************
************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include <util/delay.h>
#include "libraries/lib_mcu/can/can_lib.h"
#include <stdio.h>

#define UART_BAUDRATE 9600UL
#define BAUD_PRESCALE ((F_CPU / (UART_BAUDRATE * 16UL))-1)

#define MIN_THROTTLE	3650
#define MAX_THROTTLE	2050

#define CLUTCH_ON 0
#define CLUTCH_OFF 1
#define CLUTCH_CW 0 //For Setting direction facing motor
#define CLUTCH_CCW 1 //For Setting direction facing motor

// setup and initialize UART for debugging messages
void serial_init()
{
    UCSR0C |= (0<<UMSEL0);				//asynchronous operation
    UCSR0A |= (0<<U2X0);				//normal asynchronous mode
    UBRR0H |= (unsigned char) (BAUD_PRESCALE>>8);	//set the baud rate (high register)
    UBRR0L |= (unsigned char) BAUD_PRESCALE;	//set the baud rate (low register)
    UCSR0C |= (0<<UPM01)|(0<<UPM00);		//no parity bits
    UCSR0C |= (0<<USBS0);				//one stop bit
    UCSR0C |= (0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ01);	//8-bit character size
    UCSR0B |= (1<<TXEN0);				//enable transmitter
    return;
}

void GPIO_init(){
    
    //SET REG to OUTPUT
    DDRA |= (1<<PA1); //For Clutch_Motor On/off
    DDRA |= (1<<PA2); //For Clutch_Motor Direction

    //SET REG to INPUT
    DDRD  &= ~(1<<PD0); //Used for INT0 (interupt for encoder_count)
    PORTD &= ~(1<<PD0); //Disable pullup resistor for PD0

}

// functions for sending debug messages over serial bus
void send_char(char data)
{
	// wait for transmit buffer to be empty
    while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;	// write to transmit buffer
    return;
}

void send_message(char* message)
{
	//while there are still characters to send
    while(*message |= '\0')
    {
        send_char(*message);
        message++;
    }
    send_char('\n');
    send_char('\r');
    return;
}

void send_string(char* message)
{ 
	//while there are still characters to send
    while(*message |= '\0')
    {
        send_char(*message);
        message++;
    }
    
    return;
}

// initialize pwm for throttle
void send_int(U16 data)
{
    // wait for transmit buffer to be empty
    while(!(UCSR0A & (1<<UDRE0)));
    char str[7];
    sprintf(str,"%d",data);
    send_string(str);
    return;
}

// OC1A is the pin outputtng the pwm signal
void pwm_init()
{
    DDRB |= (1<<PB5);	// set as output
    TCCR1B |= (0<<CS12)|(1<<CS11)|(0<<CS10);	//set prescalar
    TCCR1A |= (1<<COM1A1)|(0<<COM1A0);	//set compare output mode
    TCCR1A |= (1<<WGM11)|(0<<WGM10);	//set output mode
    TCCR1B |= (1<<WGM13)|(1<<WGM12);
    ICR1 = 39999;
    OCR1A = 20000;
    return;
}

void set_duty(unsigned int  duty)
{
    U16 diff = MIN_THROTTLE - MAX_THROTTLE;
    U16 pos_rel_to_min = (duty)*(diff/100);
    U16 final = MIN_THROTTLE - pos_rel_to_min;
    OCR1A = final;
    return;
}

void clutch_motor_power(U8 on_off){

    switch(on_off){
        case (CLUTCH_ON):
            send_message("clutch on (function)");
            PORTA &= ~(1<<PA1);
            break;
        case(CLUTCH_OFF):
            PORTA |= (1<<PA1); //Setting bit High
            send_message("clutch off (function)");
            break;
        default:
            break;
    }

}

void clutch_dir(U8 dir){

    switch(dir){
        case (CLUTCH_CW):
            PORTA &= ~(1<<PA2);
            break;
        case(CLUTCH_CCW):
            PORTA |= (1<<PA2); //Setting bit High
            break;
        default:
            break;
    }
    TCNT3 = 0;
    clutch_motor_power(CLUTCH_ON); //initiallize motor on
}

void adc_init()
{
    // turn on adc
    ADMUX |= (1<<REFS0); //SET Voltage ref to internal VIN
    ADMUX |= (1<<ADLAR); //LEFT SHIFT so we just have to READ ADCH reg for data
    //DEFAULT MUX is set to ADC0
    ADCSRA |= (1<<ADEN);   //ADC Enabled
    return;
}


U16 INCREMENT_COUNT = 0;
U16 COUNT_LEN = 53;
ISR(INT0_vect)
{
    INCREMENT_COUNT++;
    send_int(INCREMENT_COUNT);
    send_string("\n\r");
    if(INCREMENT_COUNT >= COUNT_LEN)
    {
        INCREMENT_COUNT = 0;
        clutch_motor_power(CLUTCH_OFF);
        send_message("count reached");
    }
    return;
}

// set up timer for turning off motor after half second
void clutch_timer_init()
{
    TCCR3B |= (1<<CS32)|(0<<CS31)|(0<<CS30);    //set prescalar 256
    TCCR3B |= (0<<WGM33)|(0<<WGM32);    //set normal mode
    TCCR3A |= (0<<WGM31)|(0<<WGM30);
    TIMSK3 |= (1<<OCIE3A); // enable interrupt on compare match
    OCR3A = 37500;
}

ISR(TIMER3_COMPA_vect)
{
    send_message("clutch off (timeout)");
    clutch_motor_power(CLUTCH_OFF);
    INCREMENT_COUNT = 0;
}

void sendCanMessage(U8* data, U16 id, U8 datalen)
{
    //variables for sending CAN
    st_cmd_t message;
    message.cmd = CMD_TX;
    message.id.std = id;
    message.dlc = datalen;
    message.pt_data = data;
    while(can_cmd(&message) != CAN_CMD_ACCEPTED);
    while(can_get_status(&message) == CAN_STATUS_NOT_COMPLETED);
    return;
}

void recieveCanMessage(U8* buf, st_cmd_t* message)
{
    U8 c_status;
    message->pt_data = buf;
    message->cmd = CMD_RX;
    while(can_cmd(message) != CAN_CMD_ACCEPTED);
    while(can_get_status(message) == CAN_STATUS_NOT_COMPLETED);

    // if command not accepted
    /*if(can_cmd(message) == CAN_CMD_REFUSED)
    {
        return;
    }
    else
    {
        c_status = can_get_status(message);
        switch(c_status)
        {
            case CAN_STATUS_COMPLETED:
                break;
            case CAN_STATUS_NOT_COMPLETED:
                break;
            default:
                break;
        }
    }*/

    return;
}

// wait for bootup message
// return 1 if successful, 0 otherwise
int waitForBootupMessage()
{
    U8 buf[8];
    U32 count = 0;
    st_cmd_t message;

    send_message("Wait for bootup message:");
    while(count++ <= 100)
    {
        recieveCanMessage(buf, &message);

    /*    // debug for messages for can line
        send_string("id:\t");
        send_int(message.id.std);
        send_string("\tMessage:\t");
        for(int i = 0; i<message.dlc;i++)
        {
            send_char((*message.pt_data)[i]);
            send_char('\t');
        }
        send_string("\n\r");*/

        if((message.id.std==0x701)&&(buf[0]==0x00))
        {
            send_message("Boot up Message Recieved");
            return 1;
        }
    }
    send_message("Bootup message not recieved");
    return 0;
}

// check device is in enabled operation state
// return 1 if is in enabled operation, 0 otherwise
int waitForEnabledOperation()
{
    U8 buf[8];
    U32 count = 0;
    st_cmd_t message;

    send_message("wait for device enabled message:");
    while(count++ <= 100)
    {
        recieveCanMessage(buf, &message);
        
/*        // debug for messages for can line
        send_string("id:\t");
        send_int(message.id.std);
        send_string("\tMessage:\t");
        for(int i = 0; i<message.dlc;i++)
        {
            send_char((*message.pt_data)[i]);
            send_char('\t');
        }
        send_string("\n\r");*/

        if((message.id.std==0x281)&&(buf[0]!=0x21))
        {
            send_message("Operation Enabled");
            return 1;
        }
    }
    send_message("Device Enable operation Failed");
    _delay_ms(1000);
    return 0;
}

// initialize shifting motor
// return 1 if successful, 0 if fail
int shiftingInit()
{
    send_message("Initializing Shifting actuator:");

    // variables for sending CAN
    U8 buf[8];

    // reset controller
    send_message("Reseting Controller:");
    buf[0] = 0x82;
    buf[1] = 0x01;
    sendCanMessage(buf, 0x000, 2);

    //wait for bootup
    if(!waitForBootupMessage())
    {
        return 0; // did not revieve bootup message
    }
 
    // start canopen node
    send_message("Starting CAN node:");
    buf[0] = 0x01;
    buf[1] = 0x01;
    sendCanMessage(buf, 0x000, 2);
    _delay_ms(100);

    // shutdown output driver
    send_message("shutdown ouput driver:");
    buf[0] = 0x06;
    buf[1] = 0x00;
    sendCanMessage(buf, 0x201, 2);
    _delay_ms(100);

    // switch on output driver
    send_message("switch on output driver:");
    buf[0] = 0x07;
    buf[1] = 0x00;
    sendCanMessage(buf, 0x201, 2);
    _delay_ms(100);

    // Enable operation
    send_message("Enter enable operation mode:");
    buf[0] = 0x0F;
    buf[1] = 0x00;
    sendCanMessage(buf, 0x201, 2);
    _delay_ms(100);

    // set to profile position mode
    buf[0] = 0x4F;
    buf[1] = 0x60;
    buf[2] = 0x60;
    buf[3] = 0x00;
    buf[4] = 0x01;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    sendCanMessage(buf, 0x581, 8);
    _delay_ms(100);

    
    // check if enabled
    if(!waitForEnabledOperation())
    {
        return 0; // did not enter enabled operation
    }

    return 1; // Bootup successful
}

void main(void)
{
    // setup timer for PWM to control motor 
    adc_init();
    pwm_init();
    serial_init();
    GPIO_init(); 

    clutch_motor_power(CLUTCH_OFF); //initial motor off
    clutch_timer_init();

    //initialize can
    _delay_ms(1000);
    while(can_init(0) != 1);
    st_cmd_t message;
    U8 buf[8];
    send_message("CAN INIT successful");

    // initialize shifting
    //while(!shiftingInit());

    // incremental encoder counter setup
    //GPIO PD0 enabled as input in GPIO_init
    EIMSK |= (1 << INT0); //Set INT0 
    EICRA |= (1<<ISC00)|(1<<ISC01); // set interupt to trigger on rising edge of INT0
    
    sei(); // enable global inturrupts
    PORTD &= ~(1<<PD0); //Disable pullup resistor for PD0

    //Setting Duty Cycle Debug:
    U16 test = 255;
    send_int(OCR1A);
    send_char('\n');
    send_message("Value of OCR1A at call");
    set_duty(0);

    
    // execution loop
    while(1){
       
        // recieve message
        recieveCanMessage(buf, &message);
        
        //CAN ID specific Excution:
        switch(message.id.std)
        {
            case THROTTLE_ID:
                set_duty(buf[0]);
                break;
            case CLUTCH_ID:
                clutch_dir(buf[0]);
                break;
            case CLUTCH_COUNT:
                COUNT_LEN = buf[0];
            default:
                break;
        }//End of Switch 
    }
    return;
}
