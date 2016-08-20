/*
 * File: main.c
 * Target: PIC24F16KL401
 * IDE: MPLABX v3.35
 * Compiler: XC16 v1.26
 *
 * Description:
 *  Test UART input and output
 *
 * Operation:
 *
 * Warning:
 *
 * Pinout:
 *
 *                       PIC24F16KL401
 *              +-------------:_:-------------+
 *   ICD_VPP -> :  1 RA5/MCLR         VDD  20 : <- PWR
 *   ICD_PGC <> :  2 RA0/PGC2         VSS  19 : <- GND
 *   ICD_PGD <> :  3 RA1/PGD2    AN9 /RB15 18 : <> RED LED, HIGH=ON
 *   DBG_TXD <> :  4 RB0/TXD2   C1OUT/RB14 17 : <> 
 *   DBG_RXD <> :  5 RB1/RXD2    AN11/RB13 16 : <>
 *           <> :  6 RB2/RXD1    AN12/RB12 15 : <>
 *           <> :  7 RA2/C1INB   CCP1/RA6  14 : <> 
 *           <> :  8 RA3              RB9  13 : <>
 *           <> :  9 RB4              RB8  12 : <>
 *           <> : 10 RA4         TXD1/RB7  11 : <>
 *              +-----------------------------+
 *                          DIP-20
 *
 */
/*
 * PIC configuration words
 */
#pragma config BWRP = OFF               // Boot Segment Write Protect (Disabled)
#pragma config BSS = OFF                // Boot segment Protect (No boot flash segment)
#pragma config GWRP = OFF               // General Segment Flash Write Protect (General segment may be written)
#pragma config GSS0 = OFF               // General Segment Code Protect (No Protection)
#pragma config FNOSC = FRC              // Fast RC Oscillator (FRC)
#pragma config SOSCSRC = DIG            // Digital Mode for use with external clock on SCLKI
#pragma config LPRCSEL = LP             // Low Power/Low Accuracy
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-speed Start-up disabled))
#pragma config POSCMD = NONE            // Primary Oscillator Mode (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Pin I/O Function (Port I/O enabled (CLKO disabled))
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range (Primary Oscillator/External Clock frequency >8MHz)
#pragma config SOSCSEL = SOSCLP         // SOSC Power Selection Configuration bits (Secondary Oscillator configured for low-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching Enabled; Fail-safe Clock Monitor Disabled
#pragma config WDTPS = PS1              // Watchdog Timer Postscale Select bits (1:1)
#pragma config FWPSA = PR128            // WDT Prescaler bit (WDT prescaler ratio of 1:128)
#pragma config FWDTEN = SWON            // Watchdog Timer Enable bits (WDT controlled with SWDTEN bit setting)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected (windowed WDT disabled))
#pragma config BOREN = BOR0             // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = OFF             // Power-up Timer Enable (PWRT disabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default SCL1/SDA1 Pins for I2C1)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset at 1.8V)
#pragma config MCLRE = ON               // MCLR is the master reset input, active low
#pragma config ICS = PGx2               // ICD Pin Placement Select (EMUC/EMUD share PGC2/PGD2)
/*
 * Project header files
 */
#include <xc.h>
#include "init.h"
#include "uart.h"
/* warning non-portable function */
/*
 * This function waits for the at least the
 * specified number milliseconds then returns.
 */
void delay( unsigned long wait_ms )
{
    for (;wait_ms; --wait_ms)
    {
        asm("    repeat  %0 \n"
            "    clrwdt     \n"
            : /* no outputs */
            : "r" ((unsigned short)(FCYC/1000-14))
            );
    }
}

/*
 * main application
 */
int main( void )
    {
    register unsigned int uiTimeout;
    /*
     * Disable all interrupt sources
     */
    __builtin_disi(0x3FFF); /* disable interrupts for 16383 cycles */
    IEC0 = 0;
    IEC1 = 0;
    IEC2 = 0;
    IEC3 = 0;
    IEC4 = 0;
    IEC5 = 0;
    __builtin_disi(0x0000); /* enable interrupts */
    
    _NSTDIS = 1;    /* disable interrupt nesting */
    /*
     * Make all other GPIOs digital
     */
    ANSA = 0x0000;
    ANSB = 0x0000;
    /*
     * Make all GPIOs zero
     */
    LATA = 0x0000;
    LATB = 0x0000;
    /*
     * Setup GPIO directions
     */
    TRISA = 0xFF00;
    TRISB = 0x0000;
    /*
     * Setup system clock
     */
    CLKDIV = 0x0200;    /* select DOZE 1:1, DOZE disabled, RCDIV 0b010 (2MHz) */

    /* Disable the secondary oscillator for 32.768KHz crystal */
    __builtin_write_OSCCONL(OSCCON & ~(1<<_OSCCON_LPOSCEN_POSITION));
    
    /* Select primary oscillator as FRCDIV (0b111) */
    __builtin_write_OSCCONH(0b111);
    
    /* Request switch primary to new selection */
    __builtin_write_OSCCONL(OSCCON  | (1 << _OSCCON_OSWEN_POSITION));
    /*
     * Wait at least 60,000 instruction cycles for clock 
     * to switch then continue anyway.
     */
    for (uiTimeout=10000; --uiTimeout && OSCCONbits.OSWEN;);
    
    U2_Init();
    /*
     * This 2 second delay is here to give the ISCP device 
     * programmer a chance to reset the target a few times
     * before the main application starts up.
     * 
     * We need to remove this for the real application.
     */
    delay(2000);
    
    U2_PutString("\r\nUART Test "__DATE__", "__TIME__"\r\n");
    
    /*
     * End of main loop 
     */
    for(;;)
    {
        /* Embedded systems do not return from main */
        if (U2_HasData())
        {
            U2_PutChar (U2_GetChar());
        }
    }
}
