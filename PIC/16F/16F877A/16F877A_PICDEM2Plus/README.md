PIC16F877A Buttons and LCD interface
====================================

PIC assembly language code that runs on the Microchip PICDEM2 Plus (DM163022-1) 

http://www.microchip.com/Developmenttools/ProductDetails.aspx?PartNO=DM163022-1

The application implement a parallel interface to an Hitachi 44780 type LCD character module. The PICDEM2 Plus hardware uses 4-bit data + 3 control lines to interface with the LCD module.

The 4-bit interface on some Hitachi 44780 type LCD character module has not been implemented correctly on all of the modules that Microchip shipped with the PICDEM2 Plus. Some of the Novatek 7605 type controllers return the nibbles in the wrong order when responding to a status read. This has the consequence that the example code Microchip provides uses a naive implementation that relies on waits for all commands or data sent to the LCD module.

The LCD functions in this code probe the LCD module to detect this type of module and do a proper job of polling the LCD modules busy status.

The S2 and S3 buttons are sampled and debounced. I have reworked my PICDEM2 Plus to move button S3 from RB0 to RA5. This allows all four LEDs to be used with outputs RB3,RB2,RB1 and RB0. Updated code to drive these LEDs. Adding code to interface with the RS232 serial, I2C EEPROM, ADC input and buzzer interfaces is left to others.