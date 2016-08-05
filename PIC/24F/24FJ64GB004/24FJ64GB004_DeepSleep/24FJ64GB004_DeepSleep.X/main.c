/*  
 *     File: main.c
 *   Target: PIC24FJ64GB004
 *      IDE: MPLABX version 3.35
 * Compiler: XC16 v1.26
 *  
 * Description:
 *  Test of wake from deep sleep.
 *    Use Fast RC oscillator to output 4MHz, no PLL.
 *
 *    Target hardware is a TQFP 44-pin device in a test socket.
 *
 *    Output is to the serial port at 9600 baud, 8N1.
 *  
 *    Wake using DSWDT after 8.5 seconds or when HIGH to LOW transition on INT0 occurs.
 *  
 *                                            PIC24FJ64GB004
 *            +-----------+            +----------+            +-----------+            +-----------+
 *      <>  1 : RB9       :      <> 12 : RA10     :      <> 23 : RB2       :      <> 34 : RA4/SOSCO :
 *      <>  2 : RC6       :      <> 13 : RA7      :      <> 24 : RB3       :      <> 35 : RA9       :
 *      <>  3 : RC7       :      <> 14 : RB14     :      <> 25 : RC0       :  RXD <> 36 : RC3/U2RXD :
 *      <>  4 : RC8       :      <> 15 : RB15     :      <> 26 : RC1       :      <> 37 : RC4       :
 *  TXD <>  5 : RC9/U2TXD :  GND -> 16 : AVSS     :      <> 27 : RC2       :      <> 38 : RC5       :
 *  GND ->  6 : DISVREG   :  PWR -> 17 : AVDD     :  PWR -> 28 : VDD       :  GND -> 39 : VSS       :
 * 10uf ->  7 : VCAP      :  VPP -> 18 : MCLR     :  GND -> 29 : VSS       :  PWR -> 40 : VDD       :
 *      <>  8 : RB10/D+   :      <> 19 : RA0      :      <> 30 : RA2/OSCI  :      <> 41 : RB5       :
 *      <>  9 : RB11/D-   :      <> 20 : RA1      :      <> 31 : RA3/OSCO  :      -> 42 : VBUS      :
 *      -> 10 : VUSB      :  PGD <> 21 : RB0/PGD1 :      <> 32 : RA8       :  SW1 <> 43 : RB7/INT0  :
 *      <> 11 : RB13      :  PGC <> 22 : RB1/PGC1 :      <> 33 : RB4/SOSCI :      <> 44 : RB8       :
 *            +-----------+            +----------+            +-----------+            +-----------+
 *                                               TQFP-44
 *
 * Notes:
 *  
 */  
    
#include <xc.h>
#include <stdio.h>
    
#pragma config JTAGEN = OFF         /* JTAG port is disabled */
#pragma config GCP = OFF            /* Code protection is disabled */
#pragma config GWRP = OFF           /* Writes to program memory are allowed */
#pragma config ICS = PGx1           /* Emulator functions are shared with PGEC1/PGED1 */
#pragma config FWDTEN = ON          /* Watchdog Timer is enabled */
#pragma config WINDIS = OFF         /* Standard Watchdog Timer enabled,(Windowed-mode is disabled) */
#pragma config FWPSA = PR32         /* Prescaler ratio of 1:32 */
#pragma config WDTPS = PS8192       /* 1:8,192 */
#pragma config IESO = OFF           /* IESO mode (Two-Speed Start-up) disabled */
#pragma config FNOSC = LPRC         /* Initial Oscillator Select (Low-Power RC Oscillator (LPRC)) */
#pragma config POSCMOD = NONE       /* Primary Oscillator disabled */
#pragma config PLL96MHZ = OFF       /* 96 MHz PLL Startup is disbled start-up */
#pragma config PLLDIV = NODIV       /* Oscillator input used directly (4 MHz input) */
#pragma config FCKSM = CSECMD       /* Sw Enabled, Mon Disabled */
#pragma config OSCIOFNC = ON        /* OSCO pin functions as port I/O (RA3) */
#pragma config I2C1SEL = PRI        /* Use default SCL1/SDA1 pins for I2C1 */
#pragma config IOL1WAY = OFF        /* The IOLOCK bit can be set and cleared using the unlock sequence */
#pragma config WPEND = WPENDMEM     /* Write Protect from WPFP to the last page of memory */
#pragma config WPCFG = WPCFGDIS     /* Last page and Flash Configuration words are unprotected */
#pragma config WPDIS = WPDIS        /* Segmented code protection disabled */
#pragma config WUTSEL = LEG         /* Default regulator start-up time used */
#pragma config SOSCSEL = IO         /* SOSC pins have digital I/O functions (RA4, RB4) */
#pragma config WPFP = WPFP63        /* Highest Page (same as page 21) */
#pragma config DSWDTEN = ON         /* DSWDT enabled */
#pragma config DSBOREN = OFF        /* BOR disabled in Deep Sleep */
#pragma config RTCOSC = LPRC        /* RTCC uses Low Power RC Oscillator (LPRC) */
#pragma config DSWDTOSC = LPRC      /* DSWDT uses Low Power RC Oscillator (LPRC) */
#pragma config DSWDTPS = DSWDTPS6   /* 1:8,192 (8.5 seconds) */
    
#define FOSC        (32000000UL)
#define FCY         (FOSC/2UL)      /* Instruction Cycle Frequency */
    
#define UARTNUM     2               /* Which device UART to use */
    
#define BAUDRATE    9600L
#define USE_HI_SPEED_BRG            /* Use BRGH=1, UART high speed mode */
    
/* UART Baud Rate Calculation */
#ifdef USE_HI_SPEED_BRG
    #define BRG_DIV 4L
#else
    #define BRG_DIV 16L
#endif
    
#define BAUDRATEREG    ( ((FCY + (BRG_DIV * BAUDRATE / 2L)) / (BRG_DIV * BAUDRATE)) - 1L)
#define BAUD_ACTUAL    (FCY/BRG_DIV/(BAUDRATEREG+1))
    
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
    
/* define map input pin numbers */
enum
{   
    RPI0  = 0,      /* pin RB00 PGD1 */
    RPI1,           /* pin RB01 PGC1 */
    RPI2,           /* pin RB02 */
    RPI3,           /* pin RB03 */
    RPI4,           /* pin RB04 */
    RPI5,           /* pin RA00 */
    RPI6,           /* pin RA01 */
    RPI7,           /* pin RB07 INT0/CN23 */
    RPI8,           /* pin RB08 */
    RPI9,           /* pin RB09 */
    RPI10,          /* pin RB10 */
    RPI11,          /* pin RB11 */
    RPI13 = 13,     /* pin RB13 (REFO output)*/
    RPI14,          /* pin RB14 */
    RPI15,          /* pin RB15 */
    RPI16,          /* pin RC00 */
    RPI17,          /* pin RC01 */
    RPI18,          /* pin RC02 */
    RPI19,          /* pin RC03 */
    RPI20,          /* pin RC04 */
    RPI21,          /* pin RC05 */
    RPI22,          /* pin RC06 */
    RPI23,          /* pin RC07 */
    RPI24,          /* pin RC08 */
    RPI25,          /* pin RC09 */               
    RPI_NONE = 0x1f  
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
    RPO_OC1,        
    RPO_OC2,        
    RPO_OC3,        
    RPO_OC4,        
    RPO_OC5,        
    RPO_CTPLS=29,   
    RPO_C3OUT       
};  
    
/* Select UART as the default standard I/O port */
int __C30_UART = UARTNUM;
/*  
** 
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
    
    CLKDIV = 0x0100;    /* set for 4MHz FRC oscillator */
    
    AD1PCFG = 0xffff; /* Set for digital I/O */
    
    CM1CON  = 0x0000;
    CM2CON  = 0x0000;
    CM3CON  = 0x0000;
    
    _NSTDIS = 1;    /* disable interrupt nesting */
    
    TRISA   = 0xFFFF;
    TRISB   = 0xFFFF;
    TRISC   = 0xFFFF;
    
    /* Unlock Registers */
    __builtin_write_OSCCONL(OSCCON & 0xBF);
    
    /* unmap all inputs */
    
    _INT1R  = RPI_NONE; /* External Interrupt 1    */
    _INT2R  = RPI_NONE; /* External Interrupt 2    */
    _IC1R   = RPI_NONE; /* Input Capture 1         */
    _IC2R   = RPI_NONE; /* Input Capture 2         */
    _IC3R   = RPI_NONE; /* Input Capture 3         */
    _IC4R   = RPI_NONE; /* Input Capture 4         */
    _IC5R   = RPI_NONE; /* Input Capture 5         */
    _OCFAR  = RPI_NONE; /* Output Compare Fault A  */
    _OCFBR  = RPI_NONE; /* Output Compare Fault B  */
    _SCK1R  = RPI_NONE; /* SPI1 Clock Input        */
    _SDI1R  = RPI_NONE; /* SPI1 Data Input         */
    _SS1R   = RPI_NONE; /* SPI1 Slave Select Input */
    _SCK2R  = RPI_NONE; /* SPI2 Clock Input        */
    _SDI2R  = RPI_NONE; /* SPI2 Data Input         */
    _SS2R   = RPI_NONE; /* SPI2 Slave Select Input */
    _T2CKR  = RPI_NONE; /* Timer2 External Clock   */
    _T3CKR  = RPI_NONE; /* Timer3 External Clock   */
    _T4CKR  = RPI_NONE; /* Timer4 External Clock   */
    _T5CKR  = RPI_NONE; /* Timer5 External Clock   */
    _U1CTSR = RPI_NONE; /* UART1 Clear To Send     */
    _U1RXR  = RPI_NONE; /* UART1 Receive           */
    _U2CTSR = RPI_NONE; /* UART2 Clear To Send     */
    _U2RXR  = RPI19;    /* UART2 Receive           */ /* pin RC03 */
    
    /* unmap all outputs */
    _RP0R   = RPO_NONE; /* pin RB00 PGD1 */
    _RP1R   = RPO_NONE; /* pin RB01 PGC1 */
    _RP2R   = RPO_NONE; /* pin RB02 */
    _RP3R   = RPO_NONE; /* pin RB03 */
    _RP4R   = RPO_NONE; /* pin RB04 */
    _RP5R   = RPO_NONE; /* pin RA00 */
    _RP6R   = RPO_NONE; /* pin RA01 */
    _RP7R   = RPO_NONE; /* pin RB07 INT0/CN23 */
    _RP8R   = RPO_NONE; /* pin RB08 */
    _RP9R   = RPO_NONE; /* pin RB09 */
    _RP10R  = RPO_NONE; /* pin RB10 */
    _RP11R  = RPO_NONE; /* pin RB11 */
    _RP13R  = RPO_NONE; /* pin RB13 (REFO output)*/
    _RP14R  = RPO_NONE; /* pin RB14 */
    _RP15R  = RPO_NONE; /* pin RB15 */
    _RP16R  = RPO_NONE; /* pin RC00 */
    _RP17R  = RPO_NONE; /* pin RC01 */
    _RP18R  = RPO_NONE; /* pin RC02 */
    _RP19R  = RPO_NONE; /* pin RC03 */
    _RP20R  = RPO_NONE; /* pin RC04 */
    _RP21R  = RPO_NONE; /* pin RC05 */
    _RP22R  = RPO_NONE; /* pin RC06 */
    _RP23R  = RPO_NONE; /* pin RC07 */
    _RP24R  = RPO_NONE; /* pin RC08 */
    _RP25R  = RPO_U2TX; /* pin RC09 */ /* UART2 Transmit          */
    
    /* Lock Registers */
    __builtin_write_OSCCONL(OSCCON | 0x40);
    
    /*
     * Any GPIO pins should be configured here 
     * because a wake from deep sleep has reset 
     * them to the Power On Reset state, that 
     * is input and analog for pins used for 
     * the Analog to Digital Converter.
     */
    /* 
     * setup INT0 
     */
    CNPU2bits.CN23PUE = 1;  /* enable weak pull-up on INT0 */
    INTCON2bits.INT0EP = 1; /* negative edge */
    IPC0bits.INT0IP = 4;    /* select priority level 4 */
    IFS0bits.INT0IF = 0;    /* clear request flag */
    /*  
     * SETUP UART: No parity, one stop bit, polled
     */  
    UxMODEbits.UARTEN = 1;      /* enable uart */
    UxBRG = BAUDRATEREG;
    
#ifdef USE_HI_SPEED_BRG
    UxMODEbits.BRGH = 1;    /* use high speed mode */
#else
    UxMODEbits.BRGH = 0;    /* use low speed mode */
#endif
    
    UxSTA = 0x0400;  /* Enable TX */
    /*
     * Release deep sleep freeze of GPIO pins
     */
    DSCONbits.RELEASE = 0;
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
     * Switch from the LPOSC to a fast system oscillator.
     * 
     * In this case we will be using the FRCPLL
     */
    CLKDIV = 0x0100;    /* select DOZE 1:1, DOZE disabled, RCDIV 0b001 (4MHz), PLL disabled */

    /* Enable the PLL */
    CLKDIVbits.PLLEN = 1;

    /* Select primary oscillator as FRCPLL */
    __builtin_write_OSCCONH(0b001);
    /* Request switch primary to new selection */
    __builtin_write_OSCCONL(OSCCON  | (1 << _OSCCON_OSWEN_POSITION));

    /* wait for clock switch to complete */
    while(OSCCONbits.OSWEN); /* Warning: the simulator usually locks up here */
    
    
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
 *  Main applicaiton
 */    
int main (void)
{   
    int ResetType;
    
    /* Initialize this PIC */    
    ResetType = PIC_init();
    
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
        /* put code here */
    }
    /*
     * Enable things that can wake us
     */
    IEC0bits.INT0IE = 1;    /* enable the INT0 interrupt source */
    /*
     * Warning: Simulator does not simulate deep sleep very well
     */
    RCONbits.DPSLP = 0; /* clear all previous deep sleep wake flags */
    DSWAKE = 0;         /* clear all previous deep sleep wake flags */
    /* enter deep sleep code cut and paste from data sheet */    
    asm("disi #4");
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
