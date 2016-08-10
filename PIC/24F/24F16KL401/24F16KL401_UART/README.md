# PIC24F16KL401 UART

This application initializes the PIC to use the FRC with postscale to set the system oscillator to 2MHz for a one MIPS instruction clock.

UART2 is initialized for 9600 baud N81.

The main loop will echo characters received at UART2 back.