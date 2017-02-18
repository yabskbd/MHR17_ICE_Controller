#include "libraries/lib_mcu/can/config.h"
#include "libraries/lib_mcu/can/can_lib.h"

int main(){

	// setup
	can_init(0x0);

	U8 data;
	st_cmd_t recieve;

	recieve.pt_data = data;
	recieve.cmd = CMD_RX;

	// execution loop
	while(1) {

		while(can_cmd(recieve)==CAN_CMD_ACCEPTED) {

		}
	}

    return 0;

}
