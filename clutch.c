/************************************************
************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
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

// initialize pwm for throttle
void send_int(U16 data)
{
    // wait for transmit buffer to be empty
    while(!(UCSR0A & (1<<UDRE0)));
    char str[7];
    sprintf(str,"%d",data);
    send_message(str);
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

// set duty cycle of throttle
void set_duty(unsigned int  duty)
{
    U16 diff = MIN_THROTTLE - MAX_THROTTLE;
    //send_message("Diff between max and min \t");
    //send_int(diff);
    //send_message("Percent");
    //send_int(percent);
    U16 pos_rel_to_min = (duty)*(diff/100);
    //send_message("pos_relative to min \t");
   //send_int((int)(pos_rel_to_min));
    U16 final = MIN_THROTTLE - pos_rel_to_min;
    //send_message("Final\t");
    //send_int(final);    
    OCR1A = final;
    //send_message("OCR1A: \t");
    //send_int(OCR1A);
    return;
}

void clutch_motor_power(U8 on_off){

    switch(on_off){
        case (CLUTCH_ON):
            PORTA &= ~(1<<PA1);
            break;
        case(CLUTCH_OFF):
            PORTA |= (1<<PA1); //Setting bit High
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


U16 counter = 0;
U16 COUNT_LEN = 1;
ISR(INT0_vect)
{
    counter++;
    if(counter >= COUNT_LEN)
    {
        counter = 0;
        clutch_motor_power(CLUTCH_OFF);
    }
    return;
}

void main(void)
{
    // setup timer for PWM to control motor 
    adc_init();
    pwm_init();
    serial_init();
    GPIO_init(); 

    clutch_motor_power(CLUTCH_OFF); //initial motor off
    
    //initialize can
    while(can_init(0) != 1);
    st_cmd_t can_message;
    can_id_t can_id;
    send_message("CAN INIT successful");

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
	//End of Duty Cycle Debug */
    //

    // execution loop
    while(1){
        
        /*//Send CAN Message Seq
        send_message("Sending Message?");
        U8 data_send = 100;
        st_cmd_t toSend;
        toSend.pt_data = &data_send;
        toSend.id.std = 0x50;
        toSend.dlc = 1;
        toSend.cmd = CMD_TX_DATA;

        while(can_cmd(&toSend) != CAN_CMD_ACCEPTED);
        while(can_get_status(&toSend) == CAN_STATUS_NOT_COMPLETED);
        send_message("Sent");
        //Send CAN Message Seq End */
          
        U8 buf[8];
        //Recieved CAN  message Seq
        //send_message("Setup Recieve: \t");
        can_message.pt_data = &buf[0];
        can_message.cmd = CMD_RX_DATA;
        while(can_cmd(&can_message) != CAN_CMD_ACCEPTED){
            send_message("CAN Eror check:");
            send_int(can_get_status(&can_message));
        }
        while(can_get_status(&can_message) != CAN_STATUS_COMPLETED);
        //Recive CAN LOOP END*/
        //Check Message on the line
        //send_message("Recieved Message");
        
        //send_int(buf[0]);
        //send_int(can_message.id.std);
        
        //Recieved CAN Message Seq End */

        ///Checking Clutch state:
        //send_int(PIND & (1<<PIND0)); //Read value of PD0
        //send_int(counter);
        //send_char('\t');
        ///Clutch State debug code */
        
        //CAN ID specific Excution:
        switch(can_message.id.std)
        {
            case THROTTLE_ID:
                //send_message("Duty_CAN: \t");
                //send_int(buf[0]);
                set_duty(buf[0]);
                break;
            case CLUTCH_ID:
                //send_message("Clutching Now");                
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
