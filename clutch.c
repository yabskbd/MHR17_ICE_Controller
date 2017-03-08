/************************************************
************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "libraries/lib_mcu/can/can_lib.h"

#define UART_BAUDRATE 9600UL
#define BAUD_PRESCALE ((F_CPU / (UART_BAUDRATE * 16UL))-1)

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
	// set pwm to fast PWM mode, non-inverting compare mode, and prescalar to 8
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS01);

	// set pin to output mode
	DDRB |= (1 << PB7); 
	
	// set duty cycle of pwm
	uint8_t duty = 191; // 75% of 255 is 191
	OCR0A = duty;
	return;
}

void adc_init()
{
	// turn on adc
	ADMUX |= (1 << REFS0); //SET Voltage ref to internal VIN
    	ADMUX |= (1 << ADLAR); //LEFT SHIFT so we just have to READ ADCH reg for data
    	//DEFAULT MUX is set to ADC0
    	ADCSRA |= (1<<ADEN);   //ADC Enabled
	return;
}

//function for setting up CAN
void init_can()
{
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
	send_char('a');
	send_message("Hello World");

    	// incremental encoder counter setup
	DDRD  |= (1<<PD0);              // set PD0 as intput, used for INT0
	PORTD |= (1<<PD0);	        // enable pullup resistor
	EICRA |= (1<<ISC00)|(1<<ISC01); // set interupt to trigger on rising edge of INT0
	sei(); 				// enable global inturrupts
	
	// execution loop
	while(1)
       	{
		send_message("Go Pack Go!\n");
	}

	return;
}
