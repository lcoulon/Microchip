/*
 * File: main.c
 * Target: PIC18F27J13
 * IDE: MPLABX v3.35
 * Compiler: XC8 v1.38
 *
 * Description:
 *  This is an application to show using the
 *  deep sleep watch dog timeout to wake up
 *  after about 135 seconds.
 *
 *                             PIC18F27J13 
 *                   +--------------:_:--------------+
 *      ISCP_VPP --> :  1 MCLRn      PGD/RP10/RB7 28 : <*> ISCP_PGD
 *               < > :  2 RA0/RP0    PGC/ RP9/RB6 27 : <*> ISCP_PGC
 *               < > :  3 RA1/RP1         RP8/RB5 26 : <*> 
 *               < > :  4 RA2/AN2         RP7/RB4 25 : <*> 
 *               < > :  5 RA3/AN3         RP6/RB3 24 : < > 
 *          10uF --> :  6 VCAP      REFO/ RP5/RB2 23 : < > 
 *               < > :  7 RA5/RP2         RP4/RB1 22 : < > 
 *           GND --> :  8 VSS             RP3/RB0 21 : < > 
 *               < > :  9 RA7/OSC1            VDD 20 : <-- PWR
 *               < > : 10 RA6/OSC2            VSS 19 : <-- GND
 *               < > : 11 RC0/RP11  RXD1/RP18/RC7 18 : <*> 
 *               < > : 12 RC1/RP12  TXD1/RP17/RC6 17 : <*> 
 *               < > : 13 RC2/RP13       RP16/RC5 16 : <*> 
 *               <*> : 14 RC3/RP14       RP15/RC4 15 : <*> 
 *                   +-------------------------------+
 *                              DIP-28
 */
#define COMPILER_NOT_FOUND
    
#ifdef __XC8
#undef COMPILER_NOT_FOUND
#define COMPILER_XC8
#include <xc.h>
#else
 #ifdef __PICC18__
 #undef COMPILER_NOT_FOUND
 #define COMPILER_HTC
 #include <htc.h>
 #else
  #if __18CXX
  #undef COMPILER_NOT_FOUND
  #define COMPILER_C18
  #include <p18cxxx.h>
  #endif
 #endif
#endif
    
#ifdef COMPILER_NOT_FOUND
#error "Unknown compiler. Code builds with XC8, HTC or C18"
#endif
    
#pragma config WDTEN = OFF, PLLDIV = 2, CFGPLLEN = OFF, STVREN = ON
#pragma config XINST = OFF, CP0 = OFF, OSC = INTOSC, SOSCSEL = DIG
#pragma config CLKOEC = OFF, FCMEN = OFF, IESO = ON, WDTPS = 1024
#pragma config DSWDTOSC = INTOSCREF, RTCOSC = INTOSCREF, DSBOREN = OFF
#pragma config DSWDTEN = ON, DSWDTPS = K131, IOL1WAY = OFF, ADCSEL = BIT12
#pragma config PLLSEL = PLL96, MSSP7B_EN = MSK7, WPFP = PAGE_127
#pragma config WPCFG = OFF, WPDIS = OFF, WPEND = PAGE_WPFP
    
#define PLLX 1
#define FOSC (8000000L * PLLX)
#define FCYC (FOSC/4L)
#define _XTAL_FREQ FOSC
    
#ifdef COMPILER_C18
#pragma udata access ISR_Data
    
/*  
** Interrupt code for Microchip C18 compiler
*/  
#pragma code high_vector=0x08
#pragma interrupt C18_ISR_Handler
void C18_ISR_Handler(void) {
    /* hang forever is an interrupt asserts */
    for(;;)
    {
    }
}   
    
#pragma code /* return to the default code section */
#pragma udata /* return to the default data section */
    
#endif
    
#ifdef COMPILER_XC8
/*  
** Interrupt code for Microchip XC8 compiler
*/  
void interrupt XC8_ISR_Handler(void) {
    /* hang forever is an interrupt asserts */
    for(;;)
    {
    }
}   
    
#endif
    
#ifdef COMPILER_HTC
/*  
** Interrupt code for HI-TECH PICC-18 compiler
*/  
void interrupt HTC_ISR_Handler(void) {
    /* hang forever is an interrupt asserts */
    for(;;)
    {
    }
}   
    
#endif
    
/*  
** Initialize this PIC hardware
**  
*/  
unsigned char PIC_Init(void)
{   
    register unsigned char Result;

    enum
    {   
        eRPI_RP0 = 0,   /* pin RA0      */
        eRPI_RP1 ,      /* pin RA1      */
        eRPI_RP2 ,      /* pin RA5 SS1n */
        eRPI_RP3 ,      /* pin RB0 INT0 */
        eRPI_RP4 ,      /* pin RB1 RTCC */
        eRPI_RP5 ,      /* pin RB2 REFO */
        eRPI_RP6 ,      /* pin RB3      */
        eRPI_RP7 ,      /* pin RB4      */
        eRPI_RP8 ,      /* pin RB5      */
        eRPI_RP9 ,      /* pin RB6 PGC  */
        eRPI_RP10,      /* pin RB7 PGD  */
        eRPI_RP11,      /* pin RC0      */
        eRPI_RP12,      /* pin RC1      */
        eRPI_RP13,      /* pin RC2      */
        eRPI_RP14,      /* pin RC3 SCL1 */
        eRPI_RP15,      /* pin RC4 SDI1 */
        eRPI_RP16,      /* pin RC5 SDO1 */
        eRPI_RP17,      /* pin RC6 TX1  */
        eRPI_RP18,      /* pin RC7 RX1  */
        eRPI_NONE = 0x1F
    };  
        
    enum
    {   
        eRPO_NONE   =  0,
        eRPO_C1OUT  =  1,
        eRPO_C2OUT  =  2,
        eRPO_C3OUT  =  3,
        eRPO_TX2    =  6,
        eRPO_DT2    =  7,
        eRPO_SDO2   = 10,
        eRPO_SCK2   = 11,
        eRPO_SSDMA  = 12,
        eRPO_ULPOUT = 13,
        eRPO_CCP1   = 14,
        eRPO_CCP2   = 18,
        eRPO_CCP3   = 22,
        eRPO_P1A    = 14,
        eRPO_P1B    = 15,
        eRPO_P1C    = 16,
        eRPO_P1D    = 17,
        eRPO_P2A    = 18,
        eRPO_P2B    = 19,
        eRPO_P2C    = 20,
        eRPO_P2D    = 21,
        eRPO_P3A    = 22,
        eRPO_P3B    = 23,
        eRPO_P3C    = 24,
        eRPO_P3D    = 25
    };  
    
    INTCON = 0; /* Disable all interrupt sources */
    INTCON3 = 0;
    PIE1 = 0;
    PIE2 = 0;
    PIE3 = 0;
    PIE4 = 0;
    PIE5 = 0;
    RCONbits.IPEN = 0;          /* Use legacy interrupt priority model */
    
    OSCCON        = 0b01110000; /* Set internal oscillator to 8MHz, use oscillator set by config words */
#if PLLX == 1
    OSCTUNEbits.PLLEN = 0; /* Do not use PLL */
#else
    OSCTUNEbits.PLLEN = 1; /* Use PLL */
#endif
    
    REFOCON = 0xB0; /* Output the primary oscillator frequency on REFO(RB2) */
    
    ANCON0        = 0b11111111; /* turn off all ADC inputs */
    ANCON1        = 0b00011111;
    
    CM1CON        = 0b00000000; /* Turn off all comparators */
    CM2CON        = 0b00000000;
    CM3CON        = 0b00000000;
    CVRCON        = 0b00000000;
    
/* UnLock Registers */
    PPSCONbits.IOLOCK = 0;  /* Trick compiler to load PPSCON bank early */
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;  /* This should be a sinlge instruction, check generated code */
/* Unlock ends */
    
    /* map inputs */
    RPINR1  = eRPI_NONE;    /* INT1         */
    RPINR2  = eRPI_NONE;    /* INT2         */
    RPINR3  = eRPI_NONE;    /* INT3         */
    RPINR4  = eRPI_NONE;    /* T0CLKI       */
    RPINR6  = eRPI_NONE;    /* T3CKI        */
    RPINR7  = eRPI_NONE;    /* CCP1         */
    RPINR8  = eRPI_NONE;    /* CCP2         */
    RPINR9  = eRPI_NONE;    /* CCP3         */
    RPINR12 = eRPI_NONE;    /* T1G          */
    RPINR13 = eRPI_NONE;    /* T3G          */
    RPINR14 = eRPI_NONE;    /* T5G          */
    RPINR15 = eRPI_NONE;    /* T5CKI        */
    RPINR16 = eRPI_NONE;    /* RX2          */
    RPINR17 = eRPI_NONE;    /* CK2          */
    RPINR21 = eRPI_NONE;    /* SDI2         */
    RPINR22 = eRPI_NONE;    /* SCK2IN       */
    RPINR23 = eRPI_NONE;    /* SS2IN        */
    RPINR24 = eRPI_NONE;    /* FLT0         */
    
    /* map outputs */
    RPOR0   = eRPO_NONE;    /* pin RA0      */
    RPOR1   = eRPO_NONE;    /* pin RA1      */
    RPOR2   = eRPO_NONE;    /* pin RA5 SS1n */
    RPOR3   = eRPO_NONE;    /* pin RB0 INT0 */
    RPOR4   = eRPO_NONE;    /* pin RB1 RTCC */
    RPOR5   = eRPO_NONE;    /* pin RB2 REFO */
    RPOR6   = eRPO_NONE;    /* pin RB3      */
    RPOR7   = eRPO_NONE;    /* pin RB4      */
    RPOR8   = eRPO_NONE;    /* pin RB5      */
    RPOR9   = eRPO_NONE;    /* pin RB6 PGC  */
    RPOR10  = eRPO_NONE;    /* pin RB7 PGD  */
    RPOR11  = eRPO_NONE;    /* pin RC0      */
    RPOR12  = eRPO_NONE;    /* pin RC1      */
    RPOR13  = eRPO_NONE;    /* pin RC2      */
    RPOR14  = eRPO_NONE;    /* pin RC3 SCL1 */
    RPOR15  = eRPO_NONE;    /* pin RC4 SDI1 */
    RPOR16  = eRPO_NONE;    /* pin RC5 SDO1 */
    RPOR17  = eRPO_NONE;    /* pin RC6 TX1  */
    RPOR18  = eRPO_NONE;    /* pin RC7 RX1  */
    
/* Lock Registers */
    PPSCONbits.IOLOCK = 0;  /* Trick compiler to load PPSCON bank early */
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;  /* This should be a sinlge instruction, check generated code */
/* Lock Registers ends */
    
    LATA          = 0;
    LATB          = PORTB;      /* RB0 & RB1 used to debug deep sleep code */
    LATC          = 0;
    
    TRISA         = 0b11111111;
    TRISB         = 0b11111100; /* RB0 & RB1 used to debug deep sleep code */
    TRISC         = 0b11111111;
    INTCON2      |= 0b10000000; /* disable PORTB pull-ups           */

    /*
     * Release deep sleep freeze of GPIO pins
     */
    DSCONLbits.RELEASE = 0;
    /*
     * Decide if we can start up the high speed system oscillator
     * 
     * But this choice depends on the hardware implementation.
     * 
     * This code assumes that this is always wanted.
     */
    {
        /* put code here */
    }
    /*
     * Look at flags to see what kind of start up this is
     */
    if(WDTCONbits.DS) /* Deep sleep wake up */
    {
        if(DSWAKEHbits.DSINT0)
        {
            Result = 1;          /* INT0 wake from deep sleep */
            DSGPR1 = DSGPR1 + 1; /* count when wake from INT0  */
        }
        else if(DSWAKELbits.DSWDT)
        {
            Result = 1;          /* Timeout wake from deep sleep */
            DSGPR0 = DSGPR0 + 1; /* count when wake from DSWDT */
        }
        else if(DSWAKELbits.DSPOR)
        {
            Result = 2;          /* MCLR wake from deep sleep */
            RCON |= 0b00111111;
            DSGPR0 = 0;
            DSGPR1 = 0;
            LATB   = 0;
        }
    }
    else              /* Other class of wake up */
    {
        if(RCONbits.NOT_PD == 0) /* Power on reset */
        {
            Result = 0;         /* assume we are a Power On reset */
            RCON |= 0b00111111;
            DSGPR0 = 0;
            DSGPR1 = 0;
            LATB   = 0;
        }
        else if(RCONbits.NOT_TO) /* Watch Dog Timeout reset */
        {
            Result = 3;
            RCONbits.NOT_TO = 1;
        }
    }

    return Result;
}   
/*  
** Application loop
*/  
void main(void)
{   
    unsigned char ResetType;
    
    /* Initialize this PIC */    
    ResetType = PIC_Init();
    
    switch (ResetType)
    {
        case 0: /* Power on reset, hello there */
            break;
        case 1: /* DSWDT timeout wake from deep sleep */
            LATB ^= 1; /* toggle RB0 on each wake from deep sleep */
            break;
        case 2: /* MCLR wake from deep sleep */
            break;
        case 3: /* WDT reset, never in deep sleep */
            break;
        default: /*  */
            break;
    };
    
    /*
     * Warning: Simulator does not simulate deep sleep very well
     */
    /* enter deep sleep code */
    OSCCONbits.IDLEN = 0;
    WDTCONbits.REGSLP = 1;
    DSCONHbits.DSEN = 1;
    Nop();
    Sleep();
    /*
     * If we get to deep sleep the only way out
     * is through the reset vector.
     * 
     * When we do not get to deep sleep we print a
     * message then hang.
     */
    LATBbits.LATB1 = 1; /* assert deep sleep entry failed */

    for(;;)
    {
        Nop();
        Nop();
        Nop();
    }
}   
