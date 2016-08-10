/*
 * File: main.c
 * Target: PIC24F16KL401
 * IDE: MPLABX v3.35
 * Compiler: XC16 v1.26
 *
 * Description:
 *
 *  Proof of concept code to place a PIC24F16KL40x controller
 *  in SLEEP with minimum current demand. The test circuit
 *  uses three bypass capacitors 10nF, 100nF and 10uF on
 *  the VDD and VSS pins. There is a red LED driven by a
 *  high output from PORTB, RB15 output.
 *
 * Operation:
 *
 *  After Power On Reset the LED is driven high for about 0.25 seconds
 *  then the SLEEP mode is invoked.
 *
 * Observations:
 *
 *  The bypass capacitance is enough to keep the PIC from reaching POR for
 *  more than 10 seconds when power is removed.
 *
 *  To demonstrate apply power and observed the LED flash then remove
 *  power for 10 seconds then apply power and observe that the LED does
 *  not flash so the VDD of the PIC did not drop below the POR threshold.
 *
 * Measurements:
 *
 *  The SLEEP mode current was measure using an HP 6.5 digit DMM
 *  using the millivolt range to measure the voltage drop across
 *  a 510 ohm resistor in series with the VDD power supply.
 *
 *  The SLEEP mode current of 0.0285 microamps was measured for one
 *  test PIC24F16KL401 target device. Sorry I only have one of these.
 *
 * Pinout:
 *
 *                       PIC24F16KL401
 *              +-------------:_:-------------+
 *   ICD_VPP -> :  1 RA5/MCLR         VDD  20 : <- PWR
 *   ICD_PGC <> :  2 RA0/PGC2         VSS  19 : <- GND
 *   ICD_PGD <> :  3 RA1/PGD2    AN9 /RB15 18 : <> RED LED, HIGH=ON
 *           <> :  4 RB0/TXD2    INT1/RB14 17 : <>
 *           <> :  5 RB1/RXD2    AN11/RB13 16 : <>
 *           <> :  6 RB2/RXD1    AN12/RB12 15 : <>
 *           <> :  7 RA2/OSCI    INT2/RA6  14 : <>
 *           <> :  8 RA3/OSCO         RB9  13 : <>
 *           <> :  9 RB4/SOSCI        RB8  12 : <>
 *           <> : 10 RA4/SOSCO   TXD1/RB7  11 : <>
 *              +-----------------------------+
 *                          DIP-20
 *
 */
#include <xc.h>
    
// FBS
#pragma config BWRP = OFF               // Boot Segment Write Protect (Disabled)
#pragma config BSS = OFF                // Boot segment Protect (No boot flash segment)
    
// FGS
#pragma config GWRP = OFF               // General Segment Flash Write Protect (General segment may be written)
#pragma config GSS0 = OFF               // General Segment Code Protect (No Protection)
    
// FOSCSEL
#pragma config FNOSC = FRC              // Fast RC Oscillator (FRC)
#pragma config SOSCSRC = DIG            // Digital Mode for use with external clock on SCLKI
#pragma config LPRCSEL = LP             // Low Power/Low Accuracy
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-speed Start-up disabled))
    
// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Mode (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Pin I/O Function (Port I/O enabled (CLKO disabled))
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range (Primary Oscillator/External Clock frequency >8MHz)
#pragma config SOSCSEL = SOSCLP         // SOSC Power Selection Configuration bits (Secondary Oscillator configured for low-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching Enabled; Fail-safe Clock Monitor Disabled
    
// FWDT
#pragma config WDTPS = PS1              // Watchdog Timer Postscale Select bits (1:1)
#pragma config FWPSA = PR128            // WDT Prescaler bit (WDT prescaler ratio of 1:128)
#pragma config FWDTEN = SWON            // Watchdog Timer Enable bits (WDT controlled with SWDTEN bit setting)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected (windowed WDT disabled))
    
// FPOR
#pragma config BOREN = BOR0             // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = OFF             // Power-up Timer Enable (PWRT disabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default SCL1/SDA1 Pins for I2C1)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset at 1.8V)
#pragma config MCLRE = ON               // MCLR Pin Enable bit (RA5 input disabled; MCLR enabled)
    
// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select (EMUC/EMUD share PGC2/PGD2)
/*
 * Define constants for how we will configure the clock
 */
#define FOSC (4000000UL)
#define FCYC (FOSC/2)
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
            : "r" ((unsigned short)(FCYC/1000-1))
            );
    }
}
/*
 * main application
 */
int main( void )
{
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
     * Make all GPIOs digital
     */
    ANSA = 0x0000;
    ANSB = 0x0000;
    /*
     * Make all GPIOs zero
     */
    LATA = 0x0000;
    LATB = 0x0000;
    /*
     * Make all GPIOs outputs
     */
    TRISA = 0x0000;
    TRISB = 0x0000;
    /*
     * Setup system clock
     */
    /* Disable the secondary oscillator for 32.768KHz crystal */
    __builtin_write_OSCCONL(OSCCON & ~(1<<_OSCCON_LPOSCEN_POSITION));
    
    /* Select primary oscillator as FRCDIV */
    __builtin_write_OSCCONH(0b111);
    
    /* Request switch primary to new selection */
    __builtin_write_OSCCONL(OSCCON  | (1 << _OSCCON_OSWEN_POSITION));
    
    CLKDIV = 0x0100;    /* select DOZE 1:1, DOZE disabled, RCDIV 0b001 (4MHz) */
    
    delay(500);
    _LATB15 = 1;
    delay(250);
    _LATB15 = 0;
    for(;;)
    {
        PMD1 = 0xFFFF;  /* Disable all peripherals */
        PMD2 = 0xFFFF;
        PMD3 = 0xFFFF;
        PMD4 = 0xFFFF;
        Sleep();
    }
}
