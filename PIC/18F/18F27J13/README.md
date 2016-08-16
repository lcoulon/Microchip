# PIC18F27J13 Deep Sleep Demo

Initialize the PIC to start with the default INTOSC at 4MHz, change to 8MHz then setup for deep sleep.

Output bits on PORTB are used to indicate status. RB0 is toggled on each DSWDT timeout, about 135 seconds. RB1 is asserted to one if deep sleep is not entered.