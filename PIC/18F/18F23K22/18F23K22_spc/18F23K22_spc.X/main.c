/*  
 * File: main.c
 * Target: PIC18F23K22
 * Compiler: XC8 1.34
 * IDE: MPLABX v3.35
 *  
 * Description:
 *
 *  Capture the pulse widths of 16 sensors inputs
 *  
 * Notes:
 *
 *  Debugging this code with the MPLABX simulator
 *  revealed a TIMER3 simulation model bug.
 *
 *  MPLABX v3.35 TIMER3 simulation model has a bug
 *  where THR3H does not hold the correct value until
 *  TMR3L is read even when TIMER3 is in 8-bit access mode.
 *
 *  First posted here: http://www.microchip.com/forums/FindPost/853615
 *
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
 * Application constants
 */
#define SENSOR_SAMPLE_SIZE 16
/*
 * global data
 */
volatile int polledSensor = 0;
volatile unsigned int sensorData[SENSOR_SAMPLE_SIZE];
/*
 * Select next sensor
 */
void triggerSensor(int polledSensor)
{
    /*
     * This is a dummy function.
     *
     * It should set the input multiplexer to
     * select the next sensor pulse input for
     * the TIMER3 gate.
     */
}
/*
 * Interrupt handlers
 */
void interrupt InterruptHandlerHigh( void )
{
    /* Note: This clumsy syntax generates better code with XC8 */
    if(PIE3bits.TMR3GIE) if(PIR3bits.TMR3GIF)
    {
        PIR3bits.TMR3GIF = 0;
        // Pulse captured. Calculate the new distance.
        // We will only measure up to 255cm.
        /*
         * Note 0:
         *
         * Looks like another simulator bug.
         * TIMER3 is congigured to use 8-bit access mode yet TMR3H does
         * not hold the correct value until after TMR3L is read.
         * 
         * This bug seems to have been fixed for MPLABX v3.35
         * 
         * Version of MPLABX between v2.35 and v3.35 han not
         * been tested. YMMV.
         */
               /* This bit of wackyness makes the       */
        //TMR3L; /* simulator run right when the value    */
               /* TMR3 is compared to is less than 257. */

        /* Note 1:
         *
         * Statement uses an integer divide in the ISR.
         * 
         * This will make a call to the fixed point math
         * library. A divide takes a lot of cycles.
         *
         * Do you really want to do this here?
         */
        sensorData[polledSensor] = ((TMR3 < 255*58) ? (TMR3 / 58) : 255);

        // Start the next measurement.
        polledSensor++;
        if(polledSensor >= SENSOR_SAMPLE_SIZE)
        {
            polledSensor = 0;
            PIE3bits.TMR3GIE = 0; /* stop polling when buffer full */
        }
        triggerSensor(polledSensor);

        TMR3 = 0; // Clear the timer.
        T3GCONbits.T3GGO_nDONE = 1; /* Re-enable single-pulse capture. */
    }
}
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
 *  Setup TIMER3
 */
void TIMER3_Init(void)
{
    PIE3bits.TMR3GIE = 0;
    TRISCbits.TRISC0 = 1;   /* make RC0 an input for the TIMER3 gate */

    T3CON  = (0<<_T3CON_TMR3ON_POSITION)
           | (0<<_T3CON_T3RD16_POSITION)
           | (0<<_T3CON_nT3SYNC_POSITION)
           | (0<<_T3CON_T3SOSCEN_POSITION )
           | (0<<_T3CON_T3CKPS_POSITION)
           | (0<<_T3CON_TMR3CS_POSITION);

    T3GCON = (0<<_T3GCON_T3GSS_POSITION)
           | (1<<_T3GCON_T3GSPM_POSITION)
           | (0<<_T3GCON_T3GGO_nDONE_POSITION)
           | (0<<_T3GCON_T3GTM_POSITION)
           | (1<<_T3GCON_T3GPOL_POSITION)
           | (1<<_T3GCON_TMR3GE_POSITION);

    TMR3 = 0;
    IPR3bits.TMR3GIP = 1;
    PIR3bits.TMR3GIF = 0;
    PIE3bits.TMR3GIE = 1;
    INTCONbits.PEIE = 1;
    T3CONbits.TMR3ON = 1;
}
/*
 * Start capture of sensor data
 */
void StartSensorCapture(void)
{
    PIE3bits.TMR3GIE = 0;
    T3GCONbits.T3GGO_nDONE = 0;
    polledSensor = 0;
    TMR3 = 0;
    T3GCONbits.T3GGO_nDONE = 1; /* Enable single-pulse capture. */
    PIE3bits.TMR3GIE = 1;
}
/*  
 * This is the main application loop
 */
void main (void)
{

    PIC_Init();
    TIMER3_Init();

    /* Enable interrupt system */
    INTCONbits.GIE = 1;

    /* Capture some sensor data */
    StartSensorCapture();

    /* wait for capture to complete */
    while(PIE3bits.TMR3GIE); 

    for(;;)
    {
    } 
}
