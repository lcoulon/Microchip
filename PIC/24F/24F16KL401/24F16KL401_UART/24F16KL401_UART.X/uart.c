/*  
**     file: uart.c
**   Target: PIC24F16KL401
**      IDE: MPLABX v3.35
** Compiler: XC16 v1.26
**  
** Description:
**  UART Driver for PIC24.
**  
** Notes:
**  This driver does not use interrupts.
**  
*/  
#include <xc.h>
#include "init.h"
#include "uart.h"
 
/*
** U1BRG register value and baudrate error calculation
*/
#if U1_BRGH_VALUE
#define U1_BRGH_SCALE 4L
#else
#define U1_BRGH_SCALE 16L
#endif

#define U1_BRGREG ( (FCYC + (U1_BRGH_SCALE * U1_BAUD)/1 )/(U1_BRGH_SCALE * U1_BAUD)-1L)

#if U1_BRGREG > 65535
#error Cannot set up UART1 for the FCYC and BAUDRATE. Correct values in init.h and uart.h files.
#endif

/*
** Check if baud error greater than 2.5 percent
*/
#if 0
    #define REAL_BAUDRATE ( FCYC / ( U1_BRGH_SCALE * ( U1_BRGREG + 1L) ) )
    #if (REAL_BAUDRATE > (U1_BAUD + (U1_BAUD * 25L) / 1000L)) || (REAL_BAUDRATE < (U1_BAUD - (U1_BAUD * 25L) / 1000L))
    #error UART baudrate error greater than 2.5 percent for the FCYC and U1_BAUD. Correct values in uart.c file.
    #endif
#endif
#undef REAL_BAUDRATE

/*
** U2BRG register value and baudrate error calculation
*/
#if U2_BRGH_VALUE
#define U2_BRGH_SCALE 4L
#else
#define U2_BRGH_SCALE 16L
#endif

#define U2_BRGREG (FCYC/(U2_BRGH_SCALE * U2_BAUD)-1L)

#if U2_BRGREG > 65535
#error Cannot set up UART2 for the FCYC and BAUDRATE. Correct values in init.h and uart.h files.
#endif

/*
** Check if baud error greater than 2.5 percent
*/
#define REAL_BAUDRATE ( FCYC / ( U2_BRGH_SCALE * ( U2_BRGREG + 1L) ) )
#if 0
    #if (REAL_BAUDRATE > (U2_BAUD + (U2_BAUD * 25L) / 1000L)) || (REAL_BAUDRATE < (U2_BAUD - (U2_BAUD * 25L) / 1000L))
    #error UART baudrate error greater than 2.5 percent for the FCYC and U2_BAUD. Correct values in uart.c file.
    #endif
#endif
#undef REAL_BAUDRATE

/*
** Declare private functions
*/
static void Generic_PutDec( void (*Ux_PutChar)(char), unsigned long Dec32 );
static void Generic_PutHex( void (*Ux_PutChar)(char), unsigned char Hex );
static void Generic_PutHexWord( void (*Ux_PutChar)(char), unsigned short Hex );
static void Generic_PutString( void (*Ux_PutChar)(char), char *pBuf );

/*
** Function: U1_Init
**
** Precondition: None.
**
** Overview: Setup UART2 module.
**
** Input: None.
**
** Output: None.
**
*/
void
U1_Init(
    void
    )
{
    /* Disable interrupts */
    _U1TXIE = 0;
    _U1RXIE = 0;
    _U1ERIE = 0;
    _U1RXIP = 0b100;
    _U1TXIP = 0b100;
    _U1ERIP = 0b100;
    /* Turn off UART */
    U1MODE = 0;
    U1STA = 0;
    /* Setup default GPIO states for UART pins */
#ifdef U1_TXD
    U1_TXD = 1;
#endif

#ifdef U1_TXD_DIR
    U1_TXD_DIR = 0;
#endif

#ifdef U1_RXD_DIR
    U1_RXD_DIR = 1;
#endif
    /* Initialize the UART */
    U1BRG = U1_BRGREG;
    _U1_BRGH = U1_BRGH_VALUE;
    _U1_UARTEN = 1;
    _U1_UTXEN  = 1;
    _U1RXIF = 0;        /* reset RX flag */
}

/*
** Function: U1_PutChar
**
** Precondition: U1_Init must be called before.
**
** Overview: Wait for free UART transmission buffer and send a byte.
**
** Input: Byte to be sent.
**
** Output: None.
**
*/
void  
U1_PutChar(
    char Ch
    )
{
    // wait for empty buffer  
    while(_U1_TRMT == 0);
      U1TXREG = Ch;
}

/*
** Function: U1_HasData
**
** Precondition: UART2Init must be called before.
**
** Overview: Check if there's a new byte in UART reception buffer.
**
** Input: None.
**
** Output: Zero if there's no new data received.
**
*/
char 
U1_HasData(
    void
    )
{
    char Temp;

    if (_U1_OERR != 0)
    {
        Temp = U1RXREG; /* clear overrun error */
        Temp = U1RXREG;
        Temp = U1RXREG;
        Temp = U1RXREG;
        Temp = U1RXREG;
        _U1_OERR = 0;
    }
    if(_U1RXIF == 1)
        return 1;
    return 0;
}

/*
** Function: U1_GetChar
**
** Precondition: U1_Init must be called before.
**
** Overview: Wait for a byte.
**
** Input: None.
**
** Output: Byte received.
**
**
*/
char 
U1_GetChar(
    void
    )
{
    char Temp;

    if (_U1_OERR != 0)
    {
        Temp = U1RXREG; /* clear overrun error */
        Temp = U1RXREG;
        Temp = U1RXREG;
        Temp = U1RXREG;
        Temp = U1RXREG;
        _U1_OERR = 0;
    }
    while(_U1RXIF == 0);
    Temp = U1RXREG;
    if (_U1_URXDA == 0)
    {
        _U1RXIF = 0;
    }
    return Temp;
}

/*
** Function: U1_PutDec
**
** Precondition: U1_Init must be called before.
**
** Overview: This function converts decimal data into a string
** and outputs it into UART.
**
** Input: Binary data.
**
** Output: None.
**
*/
void
U1_PutDec(
    unsigned int Dec
    )
{
    Generic_PutDec(&U1_PutChar,Dec);
}

void
U1_PutDecLong(
    unsigned long Dec
    )
{
    Generic_PutDec(&U2_PutChar,Dec);
}

/*
** Function: U1_PutString
**
** Precondition: U1_Init must be called before.
**
** Overview: This function sends an ASCIIZ string to UART2
**
** Input: pointer to ASCIIZ string
**
** Output: None.
**
*/

void
U1_PutString(
    char *pBuf
    )
{
    Generic_PutString(&U1_PutChar, pBuf);
}

/*
** Function: U1_PutHex
**
** Precondition: U1_Init must be called before.
**
** Overview: This function converts hexadecimal data into a string
** and outputs it into UART.
**
** Input: Binary data.
**
** Output: None.
**
*/
void
U1_PutHex(
    unsigned char Hex
    )
{
    Generic_PutHex(&U1_PutChar,Hex);
}

void
U1_PutHexWord(
    unsigned short Hex
    )
{
    Generic_PutHexWord(&U2_PutChar,Hex);
}

/*
** Function: U2_Init
**
** Precondition: None.
**
** Overview: Setup UART2 module.
**
** Input: None.
**
** Output: None.
**
*/
void
U2_Init(
    void
    )
{
    /* Disable interrupts */
    _U2TXIE = 0;
    _U2RXIE = 0;
    _U2ERIE = 0;
    _U2RXIP = 0b100;
    _U2TXIP = 0b100;
    _U2ERIP = 0b100;
    /* Turn off UART */
    U2MODE = 0;
    U2STA = 0;
    /* Setup default GPIO states for UART pins */
#ifdef U2_TXD
    U2_TXD = 1;
#endif

#ifdef U2_TXD_DIR
    U2_TXD_DIR = 0;
#endif

#ifdef U2_RXD_DIR
    U2_RXD_DIR = 1;
#endif
    /* Initialize the UART */
    U2BRG = U2_BRGREG;
    _U2_BRGH = U2_BRGH_VALUE;
    _U2_UARTEN = 1;
    _U2_UTXEN  = 1;
    _U2RXIF = 0;        /* reset RX flag */
}

/*
** Function: U2_PutChar
**
** Precondition: U2_Init must be called before.
**
** Overview: Wait for free UART transmission buffer and send a byte.
**
** Input: Byte to be sent.
**
** Output: None.
**
*/
void  
U2_PutChar(
    char Ch
    )
{
    // wait for empty buffer  
    while(_U2_TRMT == 0);
      U2TXREG = Ch;
}

/*
** Function: U2_HasData
**
** Precondition: UART2Init must be called before.
**
** Overview: Check if there's a new byte in UART reception buffer.
**
** Input: None.
**
** Output: Zero if there's no new data received.
**
*/
char 
U2_HasData(
    void
    )
{
    char Temp;

    if (_U2_OERR != 0)
    {
        Temp = U2RXREG; /* clear overrun error */
        Temp = U2RXREG;
        Temp = U2RXREG;
        Temp = U2RXREG;
        Temp = U2RXREG;
        _U2_OERR = 0;
    }
    if(_U2RXIF == 1)
        return 1;
    return 0;
}

/*
** Function: U2_GetChar
**
** Precondition: U2_Init must be called before.
**
** Overview: Wait for a byte.
**
** Input: None.
**
** Output: Byte received.
**
**
*/
char 
U2_GetChar(
    void
    )
{
    char Temp;

    if (_U2_OERR != 0)
    {
        Temp = U2RXREG; /* clear overrun error */
        Temp = U2RXREG;
        Temp = U2RXREG;
        Temp = U2RXREG;
        Temp = U2RXREG;
        _U2_OERR = 0;
    }
    while(_U2RXIF == 0);
    Temp = U2RXREG;
    if (_U2_URXDA == 0)
    {
        _U2RXIF = 0;
    }
    return Temp;
}

/*
** Function: U2_PutDec
**
** Precondition: U2_Init must be called before.
**
** Overview: This function converts decimal data into a string
** and outputs it into UART.
**
** Input: Binary data.
**
** Output: None.
**
*/
void
U2_PutDec(
    unsigned int Dec
    )
{
    Generic_PutDec(&U2_PutChar,(unsigned long)Dec);
}

void
U2_PutDecLong(
    unsigned long Dec
    )
{
    Generic_PutDec(&U2_PutChar,Dec);
}

/*
** Function: U2_PutString
**
** Precondition: U2_Init must be called before.
**
** Overview: This function sends an ASCIIZ string to UART2
**
** Input: pointer to ASCIIZ string
**
** Output: None.
**
*/

void
U2_PutString(
    char *pBuf
    )
{
    Generic_PutString(&U2_PutChar, pBuf);
}

/*
** Function: U2_PutHex
**
** Precondition: U2_Init must be called before.
**
** Overview: This function converts hexadecimal data into a string
** and outputs it into UART.
**
** Input: Binary data.
**
** Output: None.
**
*/
void
U2_PutHex(
    unsigned char Hex
    )
{
    Generic_PutHex(&U2_PutChar,Hex);
}

void
U2_PutHexWord(
    unsigned short Hex
    )
{
    Generic_PutHexWord(&U2_PutChar,Hex);
}

/*
** Generic print decimal to UART
**
** The UART must be initialized before this function is called.
**
** Input: Pointer to PutChar function.
**        unsigned int binary value
**
** Output: Up to 5 decimal digits sent to UART
**
** Note: This function does not use divide to convert
**       from binary to decimal.
*/
static void
Generic_PutDec(
    void (*Ux_PutChar)(char),
    unsigned long Dec32
    )
{
    unsigned short Dec16;
    unsigned char Digit;
    unsigned char ZeroFlag;

    if (Ux_PutChar)
    {
        ZeroFlag = 1;
    
        Digit = '0'; 
        if (Dec32 >= 4000000000UL)
        {
            Digit += 4;
            Dec32 -= 4000000000UL;
        }
        if (Dec32 >= 2000000000UL)
        {
            Digit += 2;
            Dec32 -= 2000000000UL;
        }
        if (Dec32 >= 1000000000UL)
        {
            Digit += 1;
            Dec32 -= 1000000000UL;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');

        Digit = '0'; 
        if (Dec32 >= 800000000UL)
        {
            Digit += 8;
            Dec32 -= 800000000UL;
        }
        if (Dec32 >= 400000000UL)
        {
            Digit += 4;
            Dec32 -= 400000000UL;
        }
        if (Dec32 >= 200000000UL)
        {
            Digit += 2;
            Dec32 -= 200000000UL;
        }
        if (Dec32 >= 100000000UL)
        {
            Digit += 1;
            Dec32 -= 100000000UL;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');

        Digit = '0';
        if (Dec32 >= 80000000UL)
        {
            Digit += 8;
            Dec32 -= 80000000UL;
        }
        if (Dec32 >= 40000000UL)
        {
            Digit += 4;
            Dec32 -= 40000000UL;
        }
        if (Dec32 >= 20000000UL)
        {
            Digit += 2;
            Dec32 -= 20000000UL;
        }
        if (Dec32 >= 10000000UL)
        {
            Digit += 1;
            Dec32 -= 10000000UL;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');

        Digit = '0';
        if (Dec32 >= 8000000UL)
        {
            Digit += 8;
            Dec32 -= 8000000UL;
        }
        if (Dec32 >= 4000000UL)
        {
            Digit += 4;
            Dec32 -= 4000000UL;
        }
        if (Dec32 >= 2000000UL)
        {
            Digit += 2;
            Dec32 -= 2000000UL;
        }
        if (Dec32 >= 1000000UL)
        {
            Digit += 1;
            Dec32 -= 1000000UL;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');

        Digit = '0';
        if (Dec32 >= 800000UL)
        {
            Digit += 8;
            Dec32 -= 800000UL;
        }
        if (Dec32 >= 400000UL)
        {
            Digit += 4;
            Dec32 -= 400000UL;
        }
        if (Dec32 >= 200000UL)
        {
            Digit += 2;
            Dec32 -= 200000UL;
        }
        if (Dec32 >= 100000UL)
        {
            Digit += 1;
            Dec32 -= 100000UL;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');

        Digit = '0';
        if (Dec32 >= 80000UL)
        {
            Digit += 8;
            Dec32 -= 80000UL;
        }
        Dec16 = Dec32;
        if (Dec16 >= 40000)
        {
            Digit += 4;
            Dec16 -= 40000;
        }
        if (Dec16 >= 20000)
        {
            Digit += 2;
            Dec16 -= 20000;
        }
        if (Dec16 >= 10000)
        {
            Digit += 1;
            Dec16 -= 10000;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');
    
        Digit = '0';
        if (Dec16 >= 8000)
        {
            Digit += 8;
            Dec16 -= 8000;
        }
        if (Dec16 >= 4000)
        {
            Digit += 4;
            Dec16 -= 4000;
        }
        if (Dec16 >= 2000)
        {
            Digit += 2;
            Dec16 -= 2000;
        }
        if (Dec16 >= 1000)
        {
            Digit += 1;
            Dec16 -= 1000;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');
     
        Digit = '0';
        if (Dec16 >= 800)
        {
            Digit += 8;
            Dec16 -= 800;
        }
        if (Dec16 >= 400)
        {
            Digit += 4;
            Dec16 -= 400;
        }
        if (Dec16 >= 200)
        {
            Digit += 2;
            Dec16 -= 200;
        }
        if (Dec16 >= 100)
        {
            Digit += 1;
            Dec16 -= 100;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');
        
        Digit = '0';
        if (Dec16 >= 80)
        {
            Digit += 8;
            Dec16 -= 80;
        }
        if (Dec16 >= 40)
        {
            Digit += 4;
            Dec16 -= 40;
        }
        if (Dec16 >= 20)
        {
            Digit += 2;
            Dec16 -= 20;
        }
        if (Dec16 >= 10)
        {
            Digit += 1;
            Dec16 -= 10;
        }
        if (('0' != Digit) || (0 == ZeroFlag))
        {
            Ux_PutChar(Digit);
            ZeroFlag = 0;
        } else Ux_PutChar(' ');
        
        Ux_PutChar(Dec16+'0');
    }
}

/*
** Generic print hexadecimal to UART
**
** The UART must be initialized before this function is called.
**
** Input: Pointer to PutChar function.
**        unsigned char binary value
**
** Output: 2 hexadecimal digits sent to UART
**
*/
static void
Generic_PutHex(
    void (*Ux_PutChar)(char),
    unsigned char Hex
    )
{
    static const char HexChar[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    if (Ux_PutChar)
    {
        Ux_PutChar(HexChar[(Hex>>4) & 0x0F ]);
        Ux_PutChar(HexChar[ Hex     & 0x0F ]);
    }
}

static void
Generic_PutHexWord(
    void (*Ux_PutChar)(char),
    unsigned short Hex
    )
{
    Generic_PutHex(Ux_PutChar,(unsigned char)(Hex>>8));
    Generic_PutHex(Ux_PutChar,(unsigned char)Hex);
}
/*
** Generic print string to UART
**
** The UART must be initialized before this function is called.
**
** Input: Pointer to PutChar function.
**        Pointer to ASCIIZ string.
**
** Output: Null terminated ASCII string sent to UART
**
** Note: Pointers are validated but no check on string length.
**
*/
static void
Generic_PutString(
    void (*Ux_PutChar)(char),
    char *pBuf
    )
{
    unsigned char c;

    if ((pBuf) && (Ux_PutChar))
    {
        c = *pBuf++;
        while(c)
        {
            Ux_PutChar(c);
            c = *pBuf++;
        }
    }
}
