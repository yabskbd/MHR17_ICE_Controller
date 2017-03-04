#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "libraries/lib_mcu/can/can_lib.h"
#include "libraries/lib_mcu/uart/uart_lib.h"

// OC0A is the pin outputtng the pwm signal
void pwm_init() {

	// set pwm to fast PWM mode, non-inverting compare mode, and prescalar to 8
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS01);

	// set pin to output mode
	DDRB |= (1 << PB7); 
	
	// set duty cycle of pwm
	uint8_t duty = 191; // 75% of 255 is 191
	OCR0A = duty;
	
	return;
}

void adc_init() {

	// turn on adc
    
	ADMUX |= (1 << REFS0); //SET Voltage ref to internal VIN
    ADMUX |= (1 << ADLAR); //LEFT SHIFT so we just have to READ ADCH reg for data
    //DEFAULT MUX is set to ADC0
    ADCSRA |= (1<<ADEN); //ADC Enabled



	return;
}

//void USART_Init(unsigned int baud){
//
//    //SET baud rate
//    UBRR0H = (unsigned char) (baud >> 8);
//    UBRR0L = (unsigned char) baud;
//
//    //set Frame format: 8data, no parity & 2 stop bits
//    UCSR0C = (0 << UMSEL0) | (0<<UPM0) | (1 << USBS0) | (3 << UCSZ0);
//    //Enable Transmit and reciever
//    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
//
//}
//
//void USART0_Transmit(uint8_t data){
//
//    /*Wait for empty transmit buffer*/
//    while(!(UCSR0A & (1<<UDRE0)));
//    UCSR0B &= ~(1<<TXB80);
//    //Put data 
//    UDR0 = data;
//
//}

int counter;
ISR(INT0_vect) {
	counter++;
}

int main(void){

    // setup timer for PWM to control motor 
	pwm_init();
    adc_init();
    //Init Serial UART
    while(uart_init(UART_BAUDRATE,0x0680)!=1);

    // incremental encoder counter setup
	DDRD  |= (1<<PD0);              // set PD0 as intput, used for INT0
	PORTD |= (1<<PD0);	        // enable pullup resistor
	EICRA |= (1<<ISC00)|(1<<ISC01); // set interupt to trigger on rising edge of INT0
	sei(); 				// enable global inturrupts
	
	// execution loop
	while(1) {
        
        uart_put_string("GO Packards");
        ADCSRA |= (1<<ADSC);
        while(ADCSRA & 0x40);
        if(ADCH > 10){
            OCR0A = (ADCH)-12;
        }


	}
}
