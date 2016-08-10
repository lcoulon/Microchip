/* 
**     file: uart.h
**   Target: PIC24F16KL401
**      IDE: MPLABX v3.35
** Compiler: XC16 v1.26
**  
** Description:
**  UART Driver for PIC24.
**  
**      
*/
#ifndef UART_H
#define UART_H

/* UART1 I/O PINS */ /* must list GPIO pin */
#define U1_TXD      _LATB7
#define U1_TXD_DIR  _TRISB7
#define U1_RXD      _RB2
#define U1_RXD_DIR  _TRISB2
         
#define U1_BAUD 9600UL
#define U1_BRGH_VALUE 1

#define _U1_BRGH   _BRGH
#define _U1_UARTEN _UARTEN
#define _U1_UTXEN  _UTXEN
#define _U1_UTXBF  _UTXBF
#define _U1_TRMT   _TRMT

#define _U1_OERR   _OERR
#define _U1_URXDA  _URXDA


/* UART2 I/O PINS */
#define U2_TXD      _LATB0
#define U2_TXD_DIR  _TRISB0
#define U2_RXD      _RB1
#define U2_RXD_DIR  _TRISB1

#define U2_BAUD 9600UL
#define U2_BRGH_VALUE 1

/* U2MODE */
#define _U2_STSEL    U2MODEbits.STSEL
#define _U2_PDSEL    U2MODEbits.PDSEL
#define _U2_BRGH     U2MODEbits.BRGH
#define _U2_RXINV    U2MODEbits.RXINV
#define _U2_ABAUD    U2MODEbits.ABAUD
#define _U2_LPBACK   U2MODEbits.LPBACK
#define _U2_WAKE     U2MODEbits.WAKE
#define _U2_UEN      U2MODEbits.UEN
#define _U2_RTSMD    U2MODEbits.RTSMD
#define _U2_IREN     U2MODEbits.IREN
#define _U2_USIDL    U2MODEbits.USIDL
#define _U2_UARTEN   U2MODEbits.UARTEN
#define _U2_PDSEL0   U2MODEbits.PDSEL0
#define _U2_PDSEL1   U2MODEbits.PDSEL1
#define _U2_UEN0     U2MODEbits.UEN0
#define _U2_UEN1     U2MODEbits.UEN1

/* U2STA */
#define _U2_URXDA    U2STAbits.URXDA
#define _U2_OERR     U2STAbits.OERR
#define _U2_FERR     U2STAbits.FERR
#define _U2_PERR     U2STAbits.PERR
#define _U2_RIDLE    U2STAbits.RIDLE
#define _U2_ADDEN    U2STAbits.ADDEN
#define _U2_URXISEL  U2STAbits.URXISEL
#define _U2_TRMT     U2STAbits.TRMT
#define _U2_UTXBF    U2STAbits.UTXBF
#define _U2_UTXEN    U2STAbits.UTXEN
#define _U2_UTXBRK   U2STAbits.UTXBRK
#define _U2_UTXISEL0 U2STAbits.UTXISEL0
#define _U2_UTXINV   U2STAbits.UTXINV
#define _U2_UTXISEL1 U2STAbits.UTXISEL1
#define _U2_URXISEL0 U2STAbits.URXISEL0
#define _U2_URXISEL1 U2STAbits.URXISEL1


void
U1_Init(
    void
    );

void  
U1_PutChar(
    char Ch
    );

char 
U1_HasData(
    void
    );

char 
U1_GetChar(
    void
    );

void
U1_PutDec(
    unsigned int Dec
    );

void
U1_PutDecLong(
    unsigned long Dec
    );

void
U1_PutHex(
    unsigned char Dec
    );

void
U1_PutHexWord(
    unsigned short Hex
    );

void
U1_PutString(
    char *pBuf
    );

void
U2_Init(
    void
    );

void  
U2_PutChar(
    char Ch
    );

char 
U2_HasData(
    void
    );

char 
U2_GetChar(
    void
    );

void
U2_PutDec(
    unsigned int Dec
    );

void
U2_PutDecLong(
    unsigned long Dec
    );

void
U2_PutHex(
    unsigned char Dec
    );

void
U2_PutHexWord(
    unsigned short Hex
    );

void
U2_PutString(
    char *pBuf
    );

#endif
