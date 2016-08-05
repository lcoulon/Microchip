# PIC24FJ128GC010 deep sleep demo

Initialize the PIC to start with the LPOSC then switch to the FRCPLL and set the system oscillator to 32MHz.

Use UART2 to send messages. Send an initial POR message then attempt to enter deep sleep. Then output a message on what caused the wake from deep sleep.