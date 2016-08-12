/*
 * File:   main.c
 * Target: PIC24FJ128GA202
 * 
 * Description:
 *  Confgure a PIC24FJ128GA202 to run using the FRC
 *  as the primary clock source and set CLKDIV for
 *  8MHz as the system oscillator frequency with 
 *  the PLL disabled and secondary oscillator pins
 *  used for digital I/O.
 *
 * Created on August 10, 2016, 4:46 PM
 *
 *                       PIC24FJ128GA202
 *              +--------------:_:--------------+
 *  ICD_VPP --> :  1 MCLR                VDD 28 : <-- PWR
 *          < > :  2 RA0                 VSS 27 : <-- GND
 *          < > :  3 RA1           RP15/RB15 26 : < > 
 *  ICD_PGD < > :  4 RB0/RP0/PGD1  RP14/RB14 25 : < > 
 *  ICD_PGC < > :  5 RB1/RP1/PGC1  RP13/RB13 24 : < > 
 *          < > :  6 RB2/RP2       RP12/RB12 23 : < > 
 *          < > :  7 RB3/RP3  PGC2/RP11/RB11 22 : <*> 
 *      GND --> :  8 VSS      PGD2/RP10/RB10 21 : <*> 
 *          < > :  9 RA2/OSCI           VCAP 20 : <-- 10UF cap
 *          < > : 10 RA3/OSCO           VBAT 19 : <-- PWR
 *          -*> : 11 RB4/SOSCI/RPI4  RP9/RB9 18 : < > 
 *          -*> : 12 RA4/SOSCI       RP8/RB8 17 : <*> 
 *          --> : 13 VDD             RP7/RB7 16 : <*> 
 *          <*> : 14 RB5/PGD3   PGC3/RP6/RB6 15 : <*> 
 *              +-------------------------------+
 *                          DIP-28
 *
 * Notes: 
 *          -->         Input only pin            <--
 *          < >            GPIO pin               < >
 *          -*>  Input only pin, 5 volt tolerant  <*-
 *          <*>     GPIO pin, 5 volt tolerant     <*>
 *
 */
#pragma config SOSCSEL = OFF /* make secondary oscillator pins available for digital I/O */
#pragma config DSWDTPS = DSWDTPS1F, DSWDTOSC = LPRC, DSBOREN = OFF, DSWDTEN = OFF
#pragma config DSSWEN = OFF, PLLDIV = DISABLED, I2C1SEL = DISABLE, IOL1WAY = OFF
#pragma config WPFP = WPFP127, WDTWIN = PS25_0, PLLSS = PLL_PRI
#pragma config BOREN = OFF, WPDIS = WPDIS, WPCFG = WPCFGDIS, WPEND = WPENDMEM
#pragma config POSCMD = NONE, WDTCLK = LPRC, OSCIOFCN = ON, FCKSM = CSECMD
#pragma config FNOSC = FRC, ALTCMPI = CxINC_RB, WDTCMX = WDTCLK, IESO = ON
#pragma config WDTPS = PS32768, FWPSA = PR128, WINDIS = OFF, FWDTEN = ON
#pragma config ICS = PGx1, LPCFG = OFF, GWRP = OFF, GCP = OFF, JTAGEN = OFF
    
#include <xc.h>
    
int main(void) 
{
    /* Select 8MHz as the FRC frequency with the PLL and DOZE disabled */
    CLKDIV = 0;
    
    /* Disable SOSC in OSCCON */
    __builtin_write_OSCCONL(OSCCON & ~(_OSCCON_SOSCEN_MASK));
    
    /* select FOSC/2 as clock for TIMER1 */
    T1CONbits.TCS = 0;
    T1CONbits.TECS = 2; /* select anything but SOSC, this is LPRC */
    
    /* select FOSC/2 as clock for TIMER2 */
    T2CONbits.TCS = 0;
    T2CONbits.TECS = 2; /* select anything but SOSC, this is LPRC */
    
    /* select FOSC/2 as clock for TIMER3 */
    T3CONbits.TCS = 0;
    T3CONbits.TECS = 2; /* select anything but SOSC, this is LPRC */
    
    /* select FOSC/2 as clock for TIMER3 */
    T4CONbits.TCS = 0;
    T4CONbits.TECS = 2; /* select anything but SOSC, this is LPRC */
    
    /* select FOSC/2 as clock for TIMER5 */
    T5CONbits.TCS = 0;
    T5CONbits.TECS = 2; /* select anything but SOSC, this is LPRC */
    
    /* make all pins digital I/O */
    ANSA = 0;
    ANSB = 0;
    
    for(;;)
    {
        /* embedded application never return from main*/
    }
    return 0;
}