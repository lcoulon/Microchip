/*  
 * File: main.c
 * Target: PIC18F23K22
 * Compiler: XC8 1.34
 * IDE: MPLABX v3.35
 *  
 * Description:
 *
 *                PIC18F23K22
 *         +---------:_:---------+
 *  RE3 -> :  1 VPP       PGD 28 : <> RB7
 *  RA0 <> :  2           PGC 27 : <> RB6
 *  RA1 <> :  3               26 : <> RB5
 *  RA2 <> :  4               25 : <> RB4
 *  RA3 <> :  5               24 : <> RB3
 *  RA4 <> :  6               23 : <> RB2
 *  RA5 <> :  7               22 : <> RB1
 *  VSS -> :  8          INT0 21 : <> RB0
 *  RA7 <> :  9 OSC1          20 : <- VDD
 *  RA6 <> : 10 OSC2          19 : <- VSS
 *  RC0 <> : 11 SOSCO         18 : <> RC7
 *  RC1 <> : 12 SOSCI         17 : <> RC6
 *  RC2 <> : 13               16 : <> RC5
 *  RC3 <> : 14               15 : <> RC4
 *         +---------------------+
 *                 DIP-28
 * 
 *  Minimal template for PIC18F23K22 as requested in post:
 *  http://www.microchip.com/forums/FindPost/941363
 * 
 *  To make matters simple: Let us output 0xFF on a port and do nothing else.
 *  What will be the configuration settings to be able to debug such a program?
 *  In Y2012 I abandoned PIC18F23K22 because no help was forthcoming.
 *  After four years I have once again decided to see if my problem can be resolved.
 *
 */  
    
#include <xc.h>
    
#pragma config FOSC = INTIO67, PLLCFG = OFF, PRICLKEN = ON, FCMEN = OFF
#pragma config IESO = OFF, PWRTEN = ON, BOREN = OFF, BORV = 220, WDTEN = OFF
#pragma config WDTPS = 32768, CCP2MX = PORTC1, PBADEN = OFF, CCP3MX = PORTC6
#pragma config HFOFST = OFF, T3CMX = PORTC0, P2BMX = PORTC0, MCLRE = EXTMCLR
#pragma config STVREN = ON, LVP = OFF, XINST = OFF
#pragma config CP0 = OFF, CP1 = OFF, CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTRB = OFF      
    
#define FOSC 16000000L
#define FCYC (FOSC/4)
/*  
 * Initialize this PIC18F23K22
 * FOSC is 16MHz derived from the internal RC oscillator at 16MHz, No PLL
 *  
 * This results in a 4MHz peripheral clock.
 */  
void PIC_Init(void)
{   
    INTCON = 0;                 /* Disable all interrupts */
    INTCON3 = 0;
    PIE1 = 0;
    PIE2 = 0;
    PIE3 = 0;
    PIE4 = 0;
    PIE5 = 0;
    
    OSCCONbits.IRCF = 0b111;    /* Internal Oscillator - HFINTOSC (16mHz) */
    OSCCONbits.SCS = 0b00;      /* System Clock Select */
    OSCTUNEbits.PLLEN = 0;      /* PLL Disable */
    
    CM1CON0bits.C1ON = 0;       /* disable everything, peripheral-wise */
    CM2CON0bits.C2ON = 0;
    VREFCON1bits.DACEN = 0;
    SLRCONbits.SLRA = 0;
    SLRCONbits.SLRB = 0;
    SRCON0bits.SRLEN = 0;
    SSP1CON1bits.SSPEN = 0;
    SSP2CON1bits.SSPEN = 0;
    
    ANSELA = 0x00;              /* Default all pins to digital */
    ANSELB = 0x00;              /* Default all pins to digital */
    ANSELC = 0x00;              /* Default all pins to digital */
    
    TRISA = 0x00;               /* Default all pins to outputs */
    TRISB = 0x00;               /* Default all pins to outputs */
    TRISC = 0x00;               /* Default all pins to outputs */
                        
    LATA = 0x00;                /* Set all pins to 0 */
    LATB = 0x00;                /* Set all pins to 0 */
    LATC = 0x00;                /* Set all pins to 0 */

    RCONbits.IPEN = 0;          /* use legacy interrupt model */
}   
/*  
 * This is the main application loop
 */
void main (void)
{

    PIC_Init();
    
    LATC = 0xFF; /* set PORTC to all ones as requested */
    
    /* hang here, embedded code never exits the main function */
    for(;;)
    {
    } 
}
