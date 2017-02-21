
#include <avr/io.h>
#include <avr/interrupt.h>
#include "libraries/lib_mcu/can/config.h"
#include "libraries/lib_mcu/can/can_lib.h"

// OC0A is the pin outputtng the pwm signal
void pwm_init() {

	// set pwm to fast PWM mode, non-inverting compare mode, and prescalar to 1
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00);

	// set pin to output mode
	DDRB |= (1 << PB7); 
	
	// set duty cycle of pwm
	uint8_t duty = 191; // 75% of 255 is 191
	OCR0A = duty;
	
	return;
}

void adc_init() {

	// turn on adc
	ADCSRA |= (1<<ADEN);

	return;
}

int counter;
ISR(INT0_vect) {
	counter++;
}

int main(void){

	// setup timer for PWM to control motor 
	pwm_init();

	// incremental encoder counter setup
	DDRD |= (1<<PD0); // set PD0 as intput, used for INT0
	PORTD |= (1<<PD0); // enable pullup resistor
	EICRA |= (1<<ISC00)|(1<<ISC01); // set interupt to trigger on rising edge of INT0
	sei(); 				// enable global inturrupts
	
	// execution loop
	while(1) {

	}
}
