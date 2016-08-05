/*  
 *     File: main.c
 *   Target: PIC24FJ128GC010
 *      IDE: MPLABX v3.35
 * Compiler: XC16 v1.26
 *  
 * Description:
 *  Test of wake from deep sleep.
 *    Use LPOSC on start from POR then switch to FRCPLL
 *    to run the system oscillator at 32MHz.
 *
 *    Target hardware is a TQFP 100-pin device in a test socket.
 *
 *    Output is to the serial port at 9600 baud, 8N1.
 *  
 *    Wake using DSWDT after 8.5 seconds or when HIGH to LOW transition on INT0 occurs.
 *  
 * Notes:
 *  
 *  I have only tried this code in the simulator so I do not know that
 *  it works in the real hardware. When I do get some PICPIC24FJ128GCxxx
 *  parts I will valiate it. Until then giv it a try and let me know
 *  what works and what fails.
 */  
    
#include <xc.h>
/* 
 * When the MPLABX editor fails to find the device specific
 * header file, This nonsense fixes the issue by including
 * a project local copy of the file.
 */
#if defined(__PIC24FJ128GC010__) && !defined(__24FJ128GC010_H)
#include "p24FJ128GC010.h"
#endif
#include <stdio.h>
    
/* CONFIG4 */
#pragma config DSWDTPS = DSWDTPSD       /* Deep Sleep Watchdog Timer Postscale Select bits (1:262114 (8.456 Secs)) */
#pragma config DSWDTOSC = LPRC          /* DSWDT Reference Clock Select (DSWDT uses LPRC as reference clock) */
#pragma config DSBOREN = OFF            /* Deep Sleep BOR Enable bit (DSBOR Disabled) */
#pragma config DSWDTEN = ON             /* Deep Sleep Watchdog Timer Enable (DSWDT Enabled) */
#pragma config DSSWEN = ON              /* DSEN Bit Enable (Deep Sleep is controlled by the register bit DSEN) */
#pragma config RTCBAT = ON              /* RTC Battery Operation Enable (RTC operation is continued through VBAT) */
#pragma config PLLDIV = DIS             /* PLL Input Prescaler Select bits (PLL is disabled) */
#pragma config I2C2SEL = PRI            /* Alternate I2C2 Location Select bit (I2C2 is multiplexed to SDA2/RA3 and SCL2/RA2 ) */
#pragma config IOL1WAY = OFF            /* PPS IOLOCK Set Only Once Enable bit (The IOLOCK bit can be set and cleared using the unlock sequence) */
    
/* CONFIG3 */
#pragma config WPFP = WPFP127           /* Write Protection Flash Page Segment Boundary (Page 127 (0x1FC00)) */
#pragma config SOSCSEL = OFF            /* SOSC Selection bits (Digital (SCLKI) mode) */
#pragma config WDTWIN = PS25_0          /* Window Mode Watchdog Timer Window Width Select (Watch Dog Timer Window Width is 25 percent) */
#pragma config BOREN = OFF              /* Brown-out Reset Enable (Brown-out Reset Disabled) */
#pragma config WPDIS = WPDIS            /* Segment Write Protection Disable (Disabled) */
#pragma config WPCFG = WPCFGDIS         /* Write Protect Configuration Page Select (Disabled) */
#pragma config WPEND = WPENDMEM         /* Segment Write Protection End Page Select (Write Protect from WPFP to the last page of memory) */
    
/* CONFIG2 */
#pragma config POSCMD = NONE            /* Primary Oscillator Select (Primary Oscillator Disabled) */
#pragma config WDTCLK = LPRC            /* WDT Clock Source Select bits (WDT uses LPRC) */
#pragma config OSCIOFCN = ON            /* OSCO/CLKO/RC15 functions as port I/O (RC15) */
#pragma config FCKSM = CSECMD           /* Clock switching is enabled, Fail-Safe Clock Monitor is disabled */
#pragma config FNOSC = LPRC             /* Initial Oscillator Select (Low-Power RC Oscillator (LPRC)) */
#pragma config ALTADREF = AVREF_RA      /* External 12-Bit A/D Reference Location Select bit (AVREF+/AVREF- are mapped to RA9/RA10) */
#pragma config ALTCVREF = CVREF_RA      /* External Comparator Reference Location Select bit (CVREF+/CVREF- are mapped to RA9/RA10) */
#pragma config WDTCMX = WDTCLK          /* WDT Clock Source Select bits (WDT clock source is determined by the WDTCLK Configuration bits) */
#pragma config IESO = OFF               /* Internal External Switchover (Disabled) */
    
/* CONFIG1 */
#pragma config WDTPS = PS32768          /* Watchdog Timer Postscaler Select (1:32,768) */
#pragma config FWPSA = PR128            /* WDT Prescaler Ratio Select (1:128) */
#pragma config WINDIS = OFF             /* Windowed WDT Disable (Standard Watchdog Timer) */
#pragma config FWDTEN = WDT_SW          /* Watchdog Timer Enable (WDT controlled with the SWDTEN bit) */
#pragma config ICS = PGx1               /* Emulator Pin Placement Select bits (Emulator functions are shared with PGEC1/PGED1) */
#pragma config LPCFG = ON               /* Low power regulator control (Low voltage regulator controlled in sw by RETEN bit) */
#pragma config GWRP = OFF               /* General Segment Write Protect (Disabled) */
#pragma config GCP = OFF                /* General Segment Code Protect (Code protection is disabled) */
#pragma config JTAGEN = OFF             /* JTAG Port Enable (Disabled) */
    
/*
 * Define the system oscillator frequency
 * 
 * Be sure that the code sets the clock up to this frequency
 */    
#define FOSC         (32000000UL)     /* System oscillator Frequency */
#define FCYC         (FOSC/2UL)       /* Instruction Cycle Frequency */
    
#define UARTNUM     2               /*Which device UART to use */
    
#define BAUDRATE    9600L
#define USE_HI_SPEED_BRG            /*Use BRGH=1, UART high speed mode */
    
/* UART Baud Rate Calculation  */
#ifdef USE_HI_SPEED_BRG
    #define BRG_DIV 4L
#else
    #define BRG_DIV 16L
#endif
    
#define BAUDRATEREG    ( ((FCYC + (BRG_DIV * BAUDRATE / 2L)) / (BRG_DIV * BAUDRATE)) - 1L)
#define BAUD_ACTUAL    (FCYC/BRG_DIV/(BAUDRATEREG+1))
    
#define BAUD_ERROR          ((BAUD_ACTUAL > BAUDRATE) ? BAUD_ACTUAL-BAUDRATE : BAUDRATE-BAUD_ACTUAL)
#define BAUD_ERROR_PRECENT  ((BAUD_ERROR*100L+BAUDRATE/2L)/BAUDRATE)
    
#if (BAUD_ERROR_PRECENT > 3)
    #error "UART frequency error is worse than 3%"
#elif (BAUD_ERROR_PRECENT > 2)
    #warning "UART frequency error is worse than 2%"
#endif
    
/* UART Configuration */
#define UARTREG2(a,b)     U##a##b
#define UARTREG(a,b)    UARTREG2(a,b)
    
#define UxMODE      UARTREG(UARTNUM,MODE)
#define UxBRG       UARTREG(UARTNUM,BRG)
#define UxSTA       UARTREG(UARTNUM,STA)
#define UxRXREG     UARTREG(UARTNUM,RXREG)
#define UxTXREG     UARTREG(UARTNUM,TXREG)
#define UxMODEbits  UARTREG(UARTNUM,MODEbits)
#define UxSTAbits   UARTREG(UARTNUM,STAbits)
#define UxTX_IO     UARTREG(UARTNUM,TX_IO)
/*
 * Select physical pins where UART2 will be mapped
 */
#define UxTX_GPIO_DIR TRISFbits.TRISF5
#define UxTX_GPIO_LAT LATFbits.LATF5
#define UxTX_GPIO_ANS ANSFbits.ANSF5
#define UxRX_GPIO_DIR TRISFbits.TRISF4
#define UxRX_GPIO_PUE CNPU2bits.CN17PUE
#define UxRX_GPIO_ANS ANSFbits.ANSF4
    
/* define map input pin numbers */
enum
{   
    RPI0  = 0,      /* pin RB0  PGD1 */
    RPI1,           /* pin RB1  PGC1 */
    RPI2,           /* pin RD8  */
    RPI3,           /* pin RD10 */
    RPI4,           /* pin RD9  */
    RPI5,           /* pin RD15 */
    RPI6,           /* pin RB6  */
    RPI7,           /* pin RB7  */
    RPI10 = 10,     /* pin RF4  */
    RPI11,          /* pin RD0  INT0/CN49 */
    RPI12,          /* pin RD11 */
    RPI13,          /* pin RB2  */
    RPI14,          /* pin RB14 */
    RPI15,          /* pin RF8  */
    RPI16,          /* pin RF3  */
    RPI17,          /* pin RF5  */
    RPI18,          /* pin RB5  */
    RPI19,          /* pin RG8  */
    RPI20,          /* pin RD5  */
    RPI21,          /* pin RG6  */
    RPI22,          /* pin RD3  */
    RPI23,          /* pin RD2  */
    RPI24,          /* pin RD1  */
    RPI25,          /* pin RD4  */               
    RPI26,          /* pin RG7  */ 
    RPI27,          /* pin RG9  */ 
    RPI28,          /* pin RB4  */
    RPI29,          /* pin RB15 (REFO output) */
    RPI30,          /* pin RF2  */
    RPI31,          /* pin RF13 */
    RPI32,          /* pin RF12 */
    RPI33,          /* pin RE8  */
    RPI34,          /* pin RE9  */
    RPI35,          /* pin RA15 */
    RPI36,          /* pin RA14 */
    RPI37,          /* pin RC14 */
    RPI38,          /* pin RC1  */
    RPI39,          /* pin RC2  */
    RPI40,          /* pin RC3  */
    RPI41,          /* pin RC4  */
    RPI42,          /* pin RD12 */
    RPI43,          /* pin RD14 */
    RPI_NONE = 0x3f  
};  
    
/* define map output function numbers */
enum
{   
    RPO_NONE = 0,
    RPO_C1OUT,      
    RPO_C2OUT,      
    RPO_U1TX,       
    RPO_U1RTS,      
    RPO_U2TX,       
    RPO_U2RTS,      
    RPO_SDO1,       
    RPO_SCK1OUT,    
    RPO_SS1OUT,     
    RPO_SDO2,       
    RPO_SCK2OUT,    
    RPO_SS2OUT,     
    RPO_OC1 = 18,        
    RPO_OC2,        
    RPO_OC3,        
    RPO_OC4,        
    RPO_OC5,        
    RPO_OC6,        
    RPO_OC7,        
    RPO_OC8,        
    RPO_U3TX = 28,
    RPO_U3RTS,
    RPO_U4TX,
    RPO_U4RTS,
    RPO_OC9 = 35,
    RPO_C3OUT,
    RPO_MDOUT
};  
/* 
 * Select UART as the default standard I/O port 
 */
int __C30_UART = UARTNUM;
/*  
 * External interrupt handler
 */  
void __attribute__((interrupt,no_auto_psv)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = 0;    /* clear request flag */
}
/*  
 * Initialize this PIC
 *  
 * Returns Power On Reset state:
 * 0 = Power On Reset
 * 1 = Deep Sleep wakeup from DSWDT timeout or INT0
 * 2 = Deep Sleep wakeup from MCLR input
 * 3 = Watchdog timeout reset, never in deep sleep
 */  
unsigned short PIC_init(void)
{   
    unsigned short Result;
    
    /* Ensure interrupts are off */
    __asm__ volatile("disi #0x3FFF");
    /* Disable all interrupts */
    IEC0 = 0;
    IEC1 = 0;
    IEC2 = 0;
    IEC3 = 0;
    IEC4 = 0;
    IEC5 = 0;
    /* Turn on interrupts */
    __asm__ volatile("disi #0x0000");

    /* Unlock Registers */
    __builtin_write_OSCCONL(OSCCON & _OSCCON_IOLOCK_MASK);
    
    /* map all inputs */
    
    _MDMINR = RPI_NONE; /* DSM Modulation Input         */
    _MDC1R  = RPI_NONE; /* DSM Carrier 1 Input          */
    _MDC2R  = RPI_NONE; /* DSM Carrier 2 Input          */
    _INT1R  = RPI_NONE; /* External Interrupt 1         */
    _INT2R  = RPI_NONE; /* External Interrupt 2         */
    _INT3R  = RPI_NONE; /* External Interrupt 3         */
    _INT4R  = RPI_NONE; /* External Interrupt 4         */
    _IC1R   = RPI_NONE; /* Input Capture 1              */
    _IC2R   = RPI_NONE; /* Input Capture 2              */
    _IC3R   = RPI_NONE; /* Input Capture 3              */
    _IC4R   = RPI_NONE; /* Input Capture 4              */
    _IC5R   = RPI_NONE; /* Input Capture 5              */
    _IC6R   = RPI_NONE; /* Input Capture 6              */
    _IC7R   = RPI_NONE; /* Input Capture 7              */
    _IC8R   = RPI_NONE; /* Input Capture 8              */
    _IC9R   = RPI_NONE; /* Input Capture 9              */
    _OCFAR  = RPI_NONE; /* Output Compare Fault A       */
    _OCFBR  = RPI_NONE; /* Output Compare Fault B       */
    _SCK1R  = RPI_NONE; /* SPI1 Clock Input             */
    _SDI1R  = RPI_NONE; /* SPI1 Data Input              */
    _SS1R   = RPI_NONE; /* SPI1 Slave Select Input      */
    _SCK2R  = RPI_NONE; /* SPI2 Clock Input             */
    _SDI2R  = RPI_NONE; /* SPI2 Data Input              */
    _SS2R   = RPI_NONE; /* SPI2 Slave Select Input      */
    _TMRCKR = RPI_NONE; /* Generic Timer External Clock */
    _U1CTSR = RPI_NONE; /* UART1 Clear-to-Send          */
    _U1RXR  = RPI_NONE; /* UART1 Receive                */
    _U2CTSR = RPI_NONE; /* UART2 Clear-to-Send          */
    _U2RXR  = RPI10;    /* UART2 Receive                */ /* pin RF4/CN17 */
    _U3CTSR = RPI_NONE; /* UART3 Clear-to-Send          */
    _U3RXR  = RPI_NONE; /* UART3 Receive                */
    _U4CTSR = RPI_NONE; /* UART4 Clear-to-Send          */
    _U4RXR  = RPI_NONE; /* UART4 Receive                */
    
    /* map all outputs */
    
    _RP0R  = RPO_NONE; /* pin RB0  PGD1 */
    _RP1R  = RPO_NONE; /* pin RB1  PGC1 */
    _RP2R  = RPO_NONE; /* pin RD8  */
    _RP3R  = RPO_NONE; /* pin RD10 */
    _RP4R  = RPO_NONE; /* pin RD9  */
    _RP5R  = RPO_NONE; /* pin RD15 */
    _RP6R  = RPO_NONE; /* pin RB6  */
    _RP7R  = RPO_NONE; /* pin RB7  */
    _RP10R = RPO_NONE; /* pin RF4  */
    _RP11R = RPO_NONE; /* pin RD0  INT0/CN23 */
    _RP12R = RPO_NONE; /* pin RD11 */
    _RP13R = RPO_NONE; /* pin RB2  */
    _RP14R = RPO_NONE; /* pin RB14 */
    _RP15R = RPO_NONE; /* pin RF8  */
    _RP16R = RPO_NONE; /* pin RF3  */
    _RP17R = RPO_U2TX; /* pin RF5  */ /* UART2 Transmit */
    _RP18R = RPO_NONE; /* pin RB5  */
    _RP19R = RPO_NONE; /* pin RG8  */
    _RP20R = RPO_NONE; /* pin RD5  */
    _RP21R = RPO_NONE; /* pin RG6  */
    _RP22R = RPO_NONE; /* pin RD3  */
    _RP23R = RPO_NONE; /* pin RD2  */
    _RP24R = RPO_NONE; /* pin RD1  */
    _RP25R = RPO_NONE; /* pin RD4  */
    _RP26R = RPO_NONE; /* pin RG7  */
    _RP27R = RPO_NONE; /* pin RG9  */
    _RP28R = RPO_NONE; /* pin RB4  */
    _RP29R = RPO_NONE; /* pin RB15 (REFO output) */
    _RP30R = RPO_NONE; /* pin RF2  */
    _RP31R = RPO_NONE; /* pin RF13 */
    
    /* Lock Registers */
    __builtin_write_OSCCONL(OSCCON | _OSCCON_IOLOCK_MASK);
    
    /*
     * Any GPIO pins should be configured here 
     * because a wake from deep sleep has reset 
     * them to the Power On Reset state, that 
     * is input and analog for pins used for 
     * the Analog to Digital Converter.
     */
    /* setup INT0 */
    ANSDbits.ANSD0 = 0;     /* Make INT0 port bit RD0 a digital input */
    TRISDbits.TRISD0 = 1;   /* Make INT0 port bit RD0 an input */
    CNPU4bits.CN49PUE = 1;  /* enable weak pull-up on INT0 */
    INTCON2bits.INT0EP = 1; /* negative edge */
    IPC0bits.INT0IP = 4;    /* select priority level 4 */
    IFS0bits.INT0IF = 0;    /* clear request flag */
    /*
     * Release deep sleep freeze
     */
    DSCONbits.RELEASE = 0;
    /*
     * Decide if we can start up the high speed system oscillator
     * 
     * But this choice depends on the hardware implementation.
     * 
     * This code assumes that this is always wanted.
     */
    
    /*
     * Switch from the LPOSC to a fast system oscillator.
     * 
     * In this case we will be using the FRCPLL
     */
    CLKDIV = 0x0100;    /* select DOZE 1:1, DOZE disabled, RCDIV 0b001 (4MHz), PLL disabled */
    /* Select primary oscillator as FRCPLL */
    __builtin_write_OSCCONH(0b001);
    
    /* Request switch primary to new selection */
    __builtin_write_OSCCONL(OSCCON  | (1 << _OSCCON_OSWEN_POSITION));

    /* wait for clock switch to complete */
    while(OSCCONbits.OSWEN); /* Warning: the simulator usually locks up here */
    
    /* start the PLL */
    CLKDIVbits.PLLEN = 1;
    
    /* check to see if we can enable the PLL */
    if(CLKDIVbits.PLLEN)
    {
        /* wait for the PLL to lock */
        while(OSCCONbits.LOCK == 0);
    }
    
    _NSTDIS = 1;    /* disable interrupt nesting */

    if(RCONbits.WDTO)
    {
        Result = 3;
        RCONbits.WDTO = 0;
    }
    else if(RCONbits.EXTR)
    {
        Result = 2;
        RCONbits.EXTR = 0;
    }
    else if(RCONbits.DPSLP)
    {
        Result = 1;
        RCONbits.DPSLP = 0;
        if(DSWAKEbits.DSWDT)  DSGPR0 = DSGPR0 + 1; /* count when wake from DSWDT */
        if(DSWAKEbits.DSINT0) DSGPR1 = DSGPR1 + 1; /* count when wake from INT0  */
    }
    else 
    {
        Result = 0;         /* assume we are a Power On reset */
        RCONbits.POR = 0;
        DSGPR0 = 0;
        DSGPR1 = 0;
    }
    return Result;
}   
/*  
 * SETUP UART: No parity, one stop bit, polled
*/  
void Uart_Init(void)
{   
    UxRX_GPIO_ANS = 0;          /* make RX port bit digital I/O */
    UxRX_GPIO_DIR = 1;          /* make RX port bit an input */
    UxRX_GPIO_PUE = 1;          /* enable pull up on RX input */
    UxTX_GPIO_ANS = 0;          /* make TX port bit digital I/O */
    UxTX_GPIO_DIR = 0;          /* make TX port bit an output */
    UxTX_GPIO_LAT = 1;          /* make TX port bit high */
    
    UxMODE =  0;                /* Setup UART mode */
    UxSTA = 0;                  /* clear UART status */
    UxMODEbits.UARTEN = 1;      /* enable UART */
    UxBRG = BAUDRATEREG;
    
#ifdef USE_HI_SPEED_BRG
    UxMODEbits.BRGH = 1;        /*use high speed mode */
#else
    UxMODEbits.BRGH = 0;        /*use low speed mode */
#endif
    
    UxSTAbits.UTXEN = 1;        /* Enable TX */
}   
/*  
 *  
*/    
int main (void)
{   
    int ResetType;
    
    /* Initialize this PIC */    
    ResetType = PIC_init();
    Uart_Init();
    
    if (ResetType == 0)
    {
        printf("\r\n  Power on reset, hello there\r\n");
    }
    else if (ResetType == 1)
    {
        printf("  DSWDT timeout wake from deep sleep %u, Wake from INT0 %u\r\n",DSGPR0,DSGPR1);
    }
    else if (ResetType == 2)
    {
        printf("  MCLR wake from deep sleep\r\n");
    }
    else if (ResetType == 3)
    {
        printf("  WDT reset, never in deep sleep\r\n");
    }
    
    while(!UxSTAbits.TRMT);
    /*
     * Turn off as much of the PIC as possible then enter deep sleep
     */
    {
    }
    /*
     * Enable things that can wake us
     */
    IEC0bits.INT0IE = 1;    /* enable the INT0 interrupt source */
    /*
     * Warning: Simulator does not simulate deep sleep very well
     */
    RCONbits.RETEN = 1; /* Enable regulator to retain RAM during Deep Sleep */
    RCONbits.DPSLP = 0; /* clear all previous deep sleep wake flags */
    DSWAKE = 0;         /* clear all previous deep sleep wake flags */
    /* enter deep sleep code cut and paste from data sheet */    
    asm("disi #5");
    asm("bset  DSCON, #15"); /* the data sheet says we need to set DSCON twice */
    asm("bset  DSCON, #15");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("pwrsav #0");
    /*
     * If we get to deep sleep the only way out
     * is through the reset vector.
     * 
     * When we do not get to deep sleep we print a
     * message then hang.
     */
    printf("  WDT wake from sleep, did not get to deep sleep\r\n");
    
    /* Main application loop */
    for(;;)
    {
    }
    
    return 0;
}   
