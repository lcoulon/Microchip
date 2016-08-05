/*
 * file: main.c
 * target: PIC18F25K80
 * IDE: MPLAB v8.92
 * Compiler: XC8 v1.38
 *
 *                   PIC18F25K80
 *             +---------:_:---------+
 *      RE3 -> :  1 VPP       PGD 28 : <> RB7
 *      RA0 <> :  2           PGC 27 : <> RB6
 *      RA1 <> :  3               26 : <> RB5
 *      RA2 <> :  4               25 : <> RB4
 *      RA3 <> :  5               24 : <> RB3
 *     VCAP <- :  6               23 : <> RB2
 *      RA5 <> :  7               22 : <> RB1
 *      VSS -> :  8          INT0 21 : <> RB0
 *      RA7 <> :  9 OSC1          20 : <- VDD
 *      RA6 <> : 10 OSC2          19 : <- VSS
 *      RC0 <> : 11 SOSCO         18 : <> RC7
 *      RC1 <> : 12 SOSCI         17 : <> RC6
 *      RC2 <> : 13               16 : <> RC5
 *      RC3 <> : 14               15 : <> RC4
 *             +---------------------+
 *                     DIP-28
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
/*
 * Include standard libs
 */
#include <string.h>

/*
 * PIC configuration words
 */
#pragma config RETEN = OFF, INTOSCSEL = HIGH, SOSCSEL = DIG, XINST = OFF
#pragma config FOSC = INTIO2, PLLCFG = OFF, FCMEN = OFF, IESO = OFF
#pragma config PWRTEN = OFF, BOREN = SBORDIS, BORV = 3, BORPWR = ZPBORMV
#pragma config WDTEN = ON, WDTPS = 1048576, CANMX = PORTB, MSSPMSK = MSK7
#pragma config MCLRE = ON, STVREN = ON, BBSIZ = BB2K
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF
#pragma config CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF
#pragma config EBTRB = OFF
/*
 * Constants Definition
 */
#define _XTAL_FREQ (64000000UL)
    
typedef struct {
    union {
        unsigned char PRI:2;
        unsigned char reserved_2:1;
        unsigned char REQ:1;
        unsigned char ERR:1;
        unsigned char LARB:1;
        unsigned char ABT:1;
        unsigned char BIF:1;
    } bits;
} TXCON_t;

typedef struct {
    union {
        struct {
            unsigned char FILHT:5;
            unsigned char RTRRO:1;
            unsigned char M1:1;
            unsigned char FUL:1;
        };
        struct {
            unsigned char FILHT0:1;
            unsigned char JTOFF:1;
            unsigned char RXB0DEN:1;
            unsigned char RXRTRRO:1;
            unsigned char reserved_3:1;
            unsigned char M0:1;
        };
    } bits;
} RXCON_t;

typedef struct {
    unsigned char SIDH;
    unsigned char SIDL;
    unsigned char EIDH;
    unsigned char EIDL;
} CanAddress_t;

typedef struct {
    union {
        TXCON_t TX;
        RXCON_t RX;
    } CON;
    CanAddress_t ID;
    union {
        struct {
            unsigned char DLC : 4;
            unsigned char RB  : 2;
            unsigned char RTR : 1;
            unsigned char reserved_7 : 1;
        } bits;
        unsigned char full;
    } DLC;
    unsigned char Data[8];
} CanBuffer;
/*
 * Declare test data and initializer function
 */
#define TXDATA_BUFFERS (3)
unsigned char TxData[TXDATA_BUFFERS][sizeof(CanBuffer)];
void TxData_Init(void)
{
    unsigned char x,y;
    for(y=0;y<TXDATA_BUFFERS;y++)
    {
        for(x=0;x<sizeof(CanBuffer);x++)
        {
            TxData[y][x] = (y<<4)+x;
        }
    }
}
/*
 * Main application
 */
void main( void )
{
    CanBuffer *pBuf;
    CanBuffer *pMsg;

    INTCON = 0;             /* Disable all interrupt sources */
    PIE1 = 0;
    PIE2 = 0;
    INTCON3bits.INT1IE = 0;
    INTCON3bits.INT2IE = 0;
    
    OSCCON = 0x70;          /* set internal oscillator to 16MHz */
    OSCTUNEbits.PLLEN = 1;  /* Turn on x4 PLL for 64MHz system clock */
    
    RCONbits.IPEN   = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.GIEH = 1;
    /*
     * Setup test data
     */
    TxData_Init();
    ((CanBuffer *)(&TxData[0][0]))->DLC.bits.DLC = 8;
    ((CanBuffer *)(&TxData[1][0]))->DLC.bits.DLC = 8;
    ((CanBuffer *)(&TxData[2][0]))->DLC.bits.DLC = 8;
    /*
     * Application loop
     */
    pBuf = (CanBuffer *)&TXB0CON;

    for(;;)
    {
        pMsg = (CanBuffer *)TxData[0];
        memcpy(pBuf->Data, (void *)pMsg->Data, pMsg->DLC.bits.DLC);
        pMsg = (CanBuffer *)TxData[1];
        memcpy(pBuf->Data, (void *)pMsg->Data, pMsg->DLC.bits.DLC);
        pMsg = (CanBuffer *)TxData[2];
        memcpy(pBuf->Data, (void *)pMsg->Data, pMsg->DLC.bits.DLC);
    }
}
