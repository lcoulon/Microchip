/*
** File: main.c
** Target: PIC32MX795F512L
** IDE: MPLABX v3.35
** Compiler: XC32 v1.42
**
** Description:
**  Use the "free" version of the XC32 compiler
**  Chase LED through 8 bits
**  Send a string of text out UART3 at 57600 baud
**  Echo characters received from UART1 to UART1
**
*/

// Adds support for PIC32 Peripheral library functions and macros
#include <GenericTypeDefs.h>
#include <plib.h>
#include <stdio.h>
#include <xc.h>

#pragma config FSRSSEL = PRIORITY_7     // SRS Select (SRS Priority 7)
#pragma config FMIIEN = OFF             // Ethernet RMII/MII Enable (RMII Enabled)
#pragma config FETHIO = OFF             // Ethernet I/O Pin Select (Alternate Ethernet I/O)
#pragma config FCANIO = OFF             // CAN I/O Pin Select (Alternate CAN I/O)
#pragma config FUSBIDIO = OFF           // USB USID Selection (Controlled by Port Function)
#pragma config FVBUSONIO = OFF          // USB VBUS ON Selection (Controlled by Port Function)
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_20         // PLL Multiplier (20x Multiplier)
#pragma config UPLLIDIV = DIV_12        // USB PLL Input Divider (12x Divider)
#pragma config UPLLEN = OFF             // USB PLL Enable (Disabled and Bypassed)
#pragma config FPLLODIV = DIV_1         // System PLL Output Clock Divider (PLL Divide by 1)
#pragma config FNOSC = FRC
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = OFF               // Internal/External Switch Over (Disabled)
#pragma config POSCMOD = OFF
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_1           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/1)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS1              // Watchdog Timer Postscaler (1:1)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config ICESEL = ICS_PGx2        // ICE/ICD Comm Channel Select (ICE EMUC2/EMUD2 pins shared with PGC2/PGD2)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)

/* System Macros */
#define GetSystemClock()        (8000000ul)
#define GetPeripheralClock()    (GetSystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()   (GetSystemClock())

/* application macros */
#define UART UART1
#define BAUD_RATE (56000ul)

void DelayMS( unsigned long Delay )
{
    unsigned long Time0, Time1;

    Time0 = _CP0_GET_COUNT();

    if (Delay < (unsigned long)(0xFFFFFFFF) / (GetSystemClock() / 1000))
        Delay = Delay * (GetSystemClock() / 1000);
    else
        Delay = (unsigned long)(0xFFFFFFFF);

    for(;;)
    {
        Time1 = _CP0_GET_COUNT();
        Time1 = Time1 - Time0;      /* Get cycle from start of spin */
        if (Time1 >= Delay)
            break;
    }
}

// *****************************************************************************
// void UARTTxBuffer(char *buffer, UINT32 size)
// *****************************************************************************
void SendDataBuffer( UART_MODULE id, const char *buffer, UINT32 size )
{
    while(size)
    {
        while(!UARTTransmitterIsReady(id))
            ;

        UARTSendDataByte(id, *buffer);

        buffer++;
        size--;
    }

    while(!UARTTransmissionHasCompleted(id))
        ;
}
static inline void __attribute__((always_inline)) UARTClearOverrun ( UART_MODULE id )
{
    uartReg[id]->sta.clr = _U1STA_OERR_MASK;
}

// *****************************************************************************
// UINT32 GetDataBuffer(char *buffer, UINT32 max_size)
// *****************************************************************************
UINT32 GetDataBuffer( UART_MODULE id, char *buffer, UINT32 max_size )
{
    UINT32 num_char;

    num_char = 0;

    while(num_char < max_size)
    {
        UINT8 character;

        for(;;)
        {
            if((UARTGetLineStatus(id) & UART_OVERRUN_ERROR) == UART_OVERRUN_ERROR)
            {
                UARTClearOverrun(id);
            }
            if(UARTReceivedDataIsAvailable(id))
            {
                break;
            }
        }
        character = UARTGetDataByte(id);

        if(character == '\r')
            break;

        *buffer = character;

        buffer++;
        num_char++;
    }

    return num_char;
}


UINT32 EchoRxTx( UART_MODULE id )
{
    UINT8 character;

    if((UARTGetLineStatus(id) & UART_OVERRUN_ERROR) == UART_OVERRUN_ERROR)
    {
        UARTClearOverrun(id);
    }
    if(UARTReceivedDataIsAvailable(id))
    {
        character = UARTGetDataByte(id);
        if(UARTTransmitterIsReady(id))
            UARTSendDataByte(id, character);
        return 1;
    }
    return 0;
}

//  port_io application code
int main(void)
{
    register unsigned int rTemp;
    UINT8   buf[1024];
    UINT32  buf_len;
    unsigned long Time0, Time1;

    // Configure the device for maximum performance, but do not change the PBDIV clock divisor.
    // Given the options, this function will change the program Flash wait states,
    // RAM wait state and enable prefetch cache, but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above.
    SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // configure IOPORTS PORTD.RD0, RD1, RD2 as outputs
    // could also use mPORTDSetPinsDigitalOut(BIT_6 | BIT_7);
    PORTSetPinsDigitalOut(IOPORT_E, BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5 | BIT_6 | BIT_7);
    PORTSetPinsDigitalOut(IOPORT_B, BIT_0 | BIT_1);
    PORTSetPinsDigitalOut(IOPORT_F, BIT_5);

    // initialize the port pins states = output low
    PORTClearBits(IOPORT_E, BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5 | BIT_6 | BIT_7);
    PORTClearBits(IOPORT_B, BIT_0 | BIT_1);
    PORTSetBits(IOPORT_F, BIT_5);

    UARTConfigure(UART, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART, GetPeripheralClock(), BAUD_RATE);
    UARTEnable(UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    /* start up delay to let MPLAB control the ICD tool */
    DelayMS(500);
    PORTSetBits(IOPORT_B, BIT_0);
    DelayMS(500);
    PORTSetBits(IOPORT_B, BIT_1);
    DelayMS(500);

    buf_len = sprintf(buf,"Debug output to UART%1d at 57600 baud\r\n",UART+1);
    SendDataBuffer(UART, buf, buf_len);
#if 0
    /* turn on +5 VDC to prototype area */
    PORTClearBits(IOPORT_F, BIT_5);
#endif
    Time0 = _CP0_GET_COUNT();
    rTemp = 1;
    LATE = 0;

    // loop and chase LED through 8 bits
    while(1)
    {
        Time1 = _CP0_GET_COUNT();
        if((EchoRxTx(UART)) || ((Time1-Time0)>=(GetSystemClock()>>1)))
        {
            Time0 = Time1;
            LATE = rTemp;
            rTemp = rTemp << 1;
            if (rTemp > 0x80)
                rTemp = 1;
        }
    }
}
