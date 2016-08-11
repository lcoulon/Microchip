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
 */
#pragma config SOSCSEL = OFF /* make secondary oscillator pins available for ditial I/O */
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
    
    /* make all pins digital I/O */
    ANSA = 0;
    ANSB = 0;
    
    for(;;)
    {
        /* embedded application never return from main*/
    }
    return 0;
}
