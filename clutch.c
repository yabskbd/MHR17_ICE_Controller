/************************************************
************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "libraries/lib_mcu/can/can_lib.h"

#define UART_BAUDRATE 9600UL
#define BAUD_PRESCALE ((F_CPU / (UART_BAUDRATE * 16UL))-1)

#define MAX_THROTTLE	4000
#define MIN_THROTTLE	1800

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
	return;
}

// OC0A is the pin outputtng the pwm signal
void pwm_init()
{
	DDRB |= 0xFF;
	TCCR1A |= (1 << COM1A1);
	TCCR1B |= (0<<CS10)|(1<<CS11)|(0<<CS12);	// set prescalar to 8
	TCCR1A |= (0<<WGM10)|(1<<WGM11);		// set to fast PWM mode, ICR1 register
	TCCR1B |= (1<<WGM12)|(1<<WGM13);		// is TOP. Toggles pin OC1A.

	// set frequency of PWM
	ICR1H |= (39999>>8);
	ICR1L |= 39999;

	// initialize duty cycle to zero
	OCR1AH = (39999/2) >> 8;
	OCR1AL = (39999/2);
	TCNT1H = 0;
	TCNT1L = 0;
	return;
}

// function for setting PWM
void set_duty(U8* duty)
{
	// calc ticks to count
	U16 duty_count = (MAX_THROTTLE-MIN_THROTTLE)*(*duty/255)+MIN_THROTTLE;
	OCR1AH |= (duty_count>>8);	//set how high to count
	OCR1AL |= duty_count;
	return;
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


int counter;
ISR(INT0_vect)
{
	counter++;
}

void main(void)
{
    	// setup timer for PWM to control motor 
	pwm_init();
    	adc_init();
	serial_init();

	//initialize can
	while(can_init(0) != 1);
	st_cmd_t can_message;
	can_id_t can_id;

    	// incremental encoder counter setup
	DDRD  |= (1<<PD0);              // set PD0 as intput, used for INT0
	PORTD |= (1<<PD0);	        // enable pullup resistor
	EICRA |= (1<<ISC00)|(1<<ISC01); // set interupt to trigger on rising edge of INT0
	sei(); 				// enable global inturrupts
	
	U8 test = 100;
	//set_duty(&test);	
	while(1);
	// execution loop
	while(1)
       	{
		// load in message
		can_message.cmd = CMD_RX_DATA;
		while(can_cmd(&can_message) != CAN_CMD_ACCEPTED);
		while(can_get_status(&can_message) != CAN_STATUS_COMPLETED);

		// check id
		switch(can_message.id.std)
		{
			case THROTTLE_ID:
				set_duty(can_message.pt_data);
				break;
			case CLUTCH_ID:
				break;
			default:
				break;
		}
	}
	return;
}
