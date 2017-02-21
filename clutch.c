
#include "libraries/lib_mcu/can/config.h"
#include "libraries/lib_mcu/can/can_lib.h"

void pwm_init() {

	// set pwm to fast PWM mode
	TCCR0A |= (1 << WGM00)|(1 << WGM01)|(1 << COM0A1)|(1 << CS00);

	// set pin to output mode
	DDRB |= (1 << PB7); 
	
	// set duty cycle of pwm
	uint8_t duty = 191; // 75% of 255 is 191
	OCR0A = duty;
	
	return;
}

int main(){

	// setup timer for PWM to control motor 
	pwm_init();

	// execution loop
	while(1) {

	}
}
