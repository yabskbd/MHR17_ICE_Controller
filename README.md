# MHR17 ICE Controller
## Code for Clutch (DC Brushless Motors) and Throttle (Servo)

## Serial
    Serial output of debuging is on TXD0(Pin 14 EXT1)
    And for Serial USB the Yellow pin is the TX
    To run screen to read Serial messages from USB line:
    screen /dev/tty.usbserial-FT94THM6 9600

## Clutch Control:
    Motor On/Off: PA1 (Pin 25 EXT1)
        01: On
        00: Off
    Motor Position: PD0 (Pin 26 EXT2@) (Uses interupt INT0)
        Make sure to disable pull after enabling sei(); global interupt
    Motor Direction Control: PA2 (Pin 26 EXT2)
        Direction defined facing motor
        CAN:
            Message: 0 CW
            Message: 1 CCW
## Throttle Motor:
    Uses PWM for servo 
    Ouput on: PB5 (Pin 29 EXT2)
    

