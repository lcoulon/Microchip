;
;  Date: 2014-MAY-5
;  File: main.asm
;  Target: PIC16F627A
;  OS: Win7 64-bit
;  MPLAB: 8.90
;  Compiler: MPASMWIN 5.49
;
;  Description: 
;   Test application for Hitachi 44780 class of LCD controller 
;   to interface a dot matrix LCD module using 4-bit mode.
;
;   The test application is an event counter. High to low
;   transitions on the TIMER0 external clock input increment
;   the event count. When the count changes line 2 of the LCD
;   is updated.
;
;  MPLAB required files:
;   P16F627A.INC
;   16F627A_G.LKR
;
    list      p=16F627A     ; list directive to define processor
    LIST      r=dec         ; list directive to set the default radix
    #include <p16F627A.inc> ; processor specific variable definitions
    errorlevel -312         ; suppress page or bank warning when building for parts with only one page


    __CONFIG   _CP_OFF & _CPD_OFF & _LVP_OFF & _BODEN_OFF & _MCLRE_ON & _WDT_OFF & _PWRTE_ON & _INTRC_OSC_NOCLKOUT 
;
; The hardware target is a PICDEM2 PLUS
; with an 18.432MHz crystal installed.
;
; Special note that the LCD module on my PICDEM2 PLUS
; is a NOVATEK 7605. In 4-bit mode the NOVATEK 7605 is 
; not 100% compatible with the Hitachi HD44780.
;
; Using the 18 pin socket for the PIC16F627A.
;
; PIC pins:             PICDEM2     LCD
; pin  1 - RA2
; pin  2 - RA3
; pin  3 - RA4          S2
; pin  4 - RA5/MCLRn    S1
; pin  5 - VSS/GND
; pin  6 - RB0/INT      S3
; pin  7 - RB1                      LCD RS
; pin  8 - RB2                      LCD R/W
; pin  9 - RB3                      LCD E
; pin 10 - RB4/PGM                  LCD D4
; pin 11 - RB5                      LCD D5
; pin 12 - RB6/PGC                  LCD D6
; pin 13 - RB7/PGD                  LCD D7
; pin 14 - VDD/+5
; pin 15 - OSC2         crystal
; pin 16 - OSC1         crystal
; pin 17 - RA0
; pin 18 - RA1

;
; LCD display module is Sharp LM20A21 
; pin  1 - VSS Ground
; pin  2 - VDD +5
; pin  3 - Vo  Contrast
; pin  4 - RS  Register select
; pin  5 - RW  Read/Write select
; pin  6 - E   Enable acrive high strobe
; pin  7 - DB0 Data bit 0
; pin  8 - DB0 Data bit 1
; pin  9 - DB0 Data bit 2
; pin 10 - DB0 Data bit 3
; pin 11 - DB0 Data bit 4
; pin 12 - DB0 Data bit 5
; pin 13 - DB0 Data bit 6
; pin 14 - DB0 Data bit 7
;
; Define the LCD port pins
#define E_PIN         PORTB,3
#define RW_PIN        PORTB,2
#define RS_PIN        PORTB,1
#define LCD_DATA_BITS 0xF0
#define LCD_PORT      PORTB
#define NOVATEK_7605
#define LCD_USE_BUSY_BIT

;
; Define macros to help with
; bank selection
;
#define BANK0  (h'000')
#define BANK1  (h'080')
#define BANK2  (h'100')
#define BANK3  (h'180')

;
; This RAM is used by the Interrupt Servoce Routine
; to save the context of the interrupted code.
INT_VAR     UDATA_SHR
w_temp      RES     1       ; variable used for context saving 
status_temp RES     1       ; variable used for context saving
pclath_temp RES     1       ; variable used for context saving

;**********************************************************************
RESET_VECTOR CODE 0x000                     ; processor reset vector
        nop                                 ; ICD2 needs this
        pagesel start
        goto    start                       ; go to beginning of program

INT_VECTOR code 0x004                       ; interrupt vector location

INTERRUPT:
        movwf   w_temp                      ; save off current W register contents
        movf    STATUS,w                    ; move status register into W register
        clrf    STATUS                      ; force to bank zero
        movwf   status_temp                 ; save off contents of STATUS register
        movf    PCLATH,W
        movwf   pclath_temp
        movlw   HIGH(INTERRUPT)
        movwf   PCLATH


; isr code can go here or be located as a called subroutine elsewhere


        movf    pclath_temp,W
        movwf   PCLATH
        movf    status_temp,w               ; retrieve copy of STATUS register
        movwf   STATUS                      ; restore pre-isr STATUS register contents
        swapf   w_temp,f
        swapf   w_temp,w                    ; restore pre-isr W register contents
        retfie                              ; return from interrupt

; -------------------------
; LCD functions
; -------------------------
;
; This code assumes a oscillator of 4MHz
;
; The the fastest oscillator a PIC16F627A can use is 20MHz.
;
; When USE_FAST_CLOCK is defined the delays are adjusted
; for a 20MHz oscillator.
;
;#define USE_FAST_CLOCK
#ifdef USE_FAST_CLOCK  
#define DELAY_FOR_FAST_CLOCK  call DelayFor18TCY
#else
#define DELAY_FOR_FAST_CLOCK
#endif


;/* Display ON/OFF Control defines */
#define DON         b'00001111'  ;/* Display on      */
#define DOFF        b'00001011'  ;/* Display off     */
#define CURSOR_ON   b'00001111'  ;/* Cursor on       */
#define CURSOR_OFF  b'00001101'  ;/* Cursor off      */
#define BLINK_ON    b'00001111'  ;/* Cursor Blink    */
#define BLINK_OFF   b'00001110'  ;/* Cursor No Blink */

;/* Cursor or Display Shift defines */
#define SHIFT_CUR_LEFT    b'00010011'  ;/* Cursor shifts to the left   */
#define SHIFT_CUR_RIGHT   b'00010111'  ;/* Cursor shifts to the right  */
#define SHIFT_DISP_LEFT   b'00011011'  ;/* Display shifts to the left  */
#define SHIFT_DISP_RIGHT  b'00011111'  ;/* Display shifts to the right */

;/* Function Set defines */
#define FOUR_BIT   b'00101111'  ;/* 4-bit Interface               */
#define EIGHT_BIT  b'00111111'  ;/* 8-bit Interface               */
#define LINE_5X7   b'00110011'  ;/* 5x7 characters, single line   */
#define LINE_5X10  b'00110111'  ;/* 5x10 characters               */
#define LINES_5X7  b'00111011'  ;/* 5x7 characters, multiple line */

; Start address of each line
#define LINE_ONE    0x00
#define LINE_TWO    0x40

;
; This RAM is used by the LCD interface routines.
;
LCD_VAR     UDATA_SHR
LCD_byte    res     1       ; byte sent to or read from LCD
pszLCD_RomStr res   2       ; pointer to ASCIIZ string in ROM

LCD_CODE code

; DelayFor18TCY() provides a 18 Tcy delay
DelayFor18TCY:
    goto    DelayFor16TCY

; DelayXLCD() provides at least 5ms delay
DelayXLCD:
;
; If we are using a fast clock make
; the delays work for a 20MHz clock.
;
#ifdef USE_FAST_CLOCK
    call    DXLCD0
    call    DXLCD0
    call    DXLCD0
    call    DXLCD0
#endif

DXLCD0:
    goto    $+1
    goto    $+1
    movlw   d'249'
DXLCD1:
    call    DelayFor16TCY
    addlw   -1
    bnz     DXLCD1
DelayFor16TCY:
    goto    $+1
    goto    $+1
    goto    $+1
    goto    $+1
    goto    $+1
    goto    $+1
    return

; DelayPORXLCD() provides at least 15ms delay
DelayPORXLCD:
    call    DelayXLCD
    call    DelayXLCD
    goto    DelayXLCD
;
; Function Name:  BusyXLCD                                  
; Return Value:   W = Not zero when status of LCD controller is busy 
; Parameters:     void                                      
; Description:    This routine reads the busy status of the 
;                 Hitachi HD44780 LCD controller.
; Notes:
;  The busy bit is not reported in the same nibble
;  on all HD44780 "compatible" controllers.
;  If you have a Novatek 7605 type controller some 
;  versions report these nibbles in reverse order.
;
;  This code has been tested with a Novatek 7605
;  and the real Hitachi HD44780.

BusyXLCD:

#ifdef LCD_USE_BUSY_BIT
    bcf     RS_PIN
    bsf     RW_PIN

    bcf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
#ifndef NOVATEK_7605
    movf    LCD_PORT,W
#endif
    bcf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
#ifdef NOVATEK_7605
    movf    LCD_PORT,W
#endif
    bcf     E_PIN
    andlw   0x80        ; the busy bit is always the MSB
    return
#endif

    call    DelayPORXLCD    ; use a 5ms delay
    movlw   0               ; instead of polling
    iorlw   0               ; the busy bit
    return
;
; Send a byte to LCD using 4-bit mode
BytePutLCD:
    banksel BANK1
    movlw   ~LCD_DATA_BITS
    andwf   LCD_PORT,F
    banksel BANK0
    andwf   LCD_PORT,F

    bcf     RW_PIN
;
; send high 4-bits
    movf    LCD_byte,W
    andlw   LCD_DATA_BITS
    iorwf   LCD_PORT,F
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
;
; send low 4-bits
    swapf   LCD_byte,W
    xorwf   LCD_byte,W
    andlw   LCD_DATA_BITS
    xorwf   LCD_PORT,F
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
; set data bits for input
    banksel BANK1
    movlw   LCD_DATA_BITS
    iorwf   LCD_PORT,F
    banksel BANK0
    return
;
; Read a byte to LCD using 4-bit mode
ByteGetLCD:
    bsf     RW_PIN
;
; read high 4-bits
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    movf    LCD_PORT,W
    bcf     E_PIN
    andlw   LCD_DATA_BITS
    movwf   LCD_byte
;
; read low 4-bits
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    swapf   LCD_PORT,W
    bcf     E_PIN
    andlw   LCD_DATA_BITS
    iorwf   LCD_byte,F
    movf    LCD_byte,W
    return
; 
; Function Name:  SetCGRamAddr                               
; Return Value:   void                                       
; Parameters:     W = character generator ram address    
; Description:    This routine sets the character generator  
;                 address of the Hitachi HD44780 LCD         
;                 controller.
;
SetCGRamAddr:
    iorlw   0x40            ; Write cmd and address to port
    movwf   LCD_byte        ; save byte going to LCD

SetCGWait:
    call    BusyXLCD
    bnz     SetCGWait

    bcf     RS_PIN
    goto    BytePutLCD

;
; Function Name:  SetDDRamAddr                              
; Return Value:   void                                      
; Parameters:     W = display data address              
; Description:    This routine sets the display data address
;                 of the Hitachi HD44780 LCD controller.
;
SetDDRamAddr:
    iorlw   0x80            ; Write cmd and address to port
    movwf   LCD_byte        ; save byte going to LCD

SetDDWait:
    call    BusyXLCD
    bnz     SetDDWait

    bcf     RS_PIN
    goto    BytePutLCD
;
; Function Name:  WriteCmdXLCD                                
; Return Value:   void                                        
; Parameters:     W = command to send to LCD                 
; Description:    This routine writes a command to the Hitachi
;                 HD44780 LCD controller.
; 
WriteCmdXLCD:
    movwf   LCD_byte        ; save byte going to LCD

WriteCmdWait:
    call    BusyXLCD
    bnz     WriteCmdWait

    bcf     RS_PIN
    goto    BytePutLCD
;
; Function Name:  WriteDataXLCD                               
; Return Value:   void                                        
; Parameters:     W = data byte to be written to LCD        
; Description:    This routine writes a data byte to the      
;                 Hitachi HD44780 LCD controller. The data  
;                 is written to the character generator RAM or
;                 the display data RAM depending on what the  
;                 previous SetxxRamAddr routine was called.   
;
WriteDataXLCD:
    movwf   LCD_byte        ; save byte going to LCD

WriteDataWait:
    call    BusyXLCD
    bnz     WriteDataWait

    bsf     RS_PIN
    call    BytePutLCD
    bcf     RS_PIN
    return
;
; Function Name:  OpenXLCD                                  
; Return Value:   void                                      
; Parameters:     W = sets the type of LCD (lines)     
; Description:    This routine configures the LCD. Based on 
;                 the Hitachi HD44780 LCD controller. The   
;                 routine will configure the I/O pins of the
;                 microcontroller, setup the LCD for 4-bit 
;                 mode and clear the display.
;
OpenXLCD:
    movwf   LCD_byte
    banksel BANK1
    movlw   ~LCD_DATA_BITS
    andwf   LCD_PORT,F
    bcf     E_PIN
    bcf     RW_PIN
    bcf     RS_PIN
    banksel BANK0
    bcf     E_PIN
    bcf     RW_PIN
    bcf     RS_PIN

    call    DelayPORXLCD    ; Wait for LCD to complete power on reset

    movlw   b'00000011'     ; force LCD into 8-bit mode
    movwf   LCD_PORT
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
    call    DelayXLCD       ; Required 5ms delay

    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
    call    DelayXLCD       ; minimum 100us delay but use 5ms

    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
    call    DelayXLCD

    movlw   b'00000010'     ; set LCD into 4-bit mode
    movwf   LCD_PORT
    bsf     E_PIN
    DELAY_FOR_FAST_CLOCK
    bcf     E_PIN
    call    DelayXLCD

    banksel BANK1
    movlw   LCD_DATA_BITS
    iorwf   LCD_PORT,F
    banksel BANK0

    movf    LCD_byte,W
    andlw   0x0F            ; Allow only 4-bit mode for
    iorlw   0x20            ; HD44780 LCD controller.
    call    WriteCmdXLCD
    
    movlw   (DOFF & CURSOR_OFF & BLINK_OFF)
    call    WriteCmdXLCD

    movlw   (DON & CURSOR_OFF & BLINK_OFF)
    call    WriteCmdXLCD

    movlw   (0x01)          ; Clear display
    call    WriteCmdXLCD

    movlw   (SHIFT_CUR_LEFT)
    call    WriteCmdXLCD

    movlw   LINE_ONE
    call    SetDDRamAddr

    return
;
; Function Name:  putrsXLCD
; Return Value:   void
; Parameters:     pszLCD_RomStr: pointer to string
; Description:    This routine writes a string of bytes to the
;                 Hitachi HD44780 LCD controller. The data
;                 is written to the character generator RAM or
;                 the display data RAM depending on what the
;                 previous SetxxRamAddr routine was called.
;
putrsXLCD:
    call    TableLookUp
    iorlw   0
    skpnz
    return
    call    WriteDataXLCD
    incf    pszLCD_RomStr,F
    skpnz
    incf    pszLCD_RomStr+1,F
    goto    putrsXLCD
    
TableLookUp:
    movfw   pszLCD_RomStr+1
    movwf   PCLATH
    movfw   pszLCD_RomStr
    movwf   PCL

START_CODE  code
;------------------------------------------------------------------------
start:
;
; Turn off all analog inputs
; and make all pins available
; for digital I/O.
; 
    banksel BANK1

    movlw   b'00000000'                 ; Turn off Comparator voltage reference
    movwf   (VRCON  ^ BANK1)

    movlw   b'11111111'                 ; Setup OPTION register
    movwf   (OPTION_REG ^ BANK1)

    banksel BANK0

    movlw   0x07                        ; turn off Comparators
    movwf   (CMCON  ^ BANK0)


    pagesel main
    goto    main

;
;------------------------------------------------------------------------
MAIN_DATA udata 0x20                    ; locate in bank0
#define EventCountDigits 16
EventCount      res EventCountDigits
EventCountIndex res 1
TMR0_Sample     res 1
Events          res 1

EVENT_CODE  code

EventCountClear:
    movlw   EventCount
    movwf   FSR
    movlw   EventCountDigits
    movwf   EventCountIndex
    movlw   '0'
ECC_1:
    movwf   INDF
    incf    FSR,F
    decfsz  EventCountIndex,F
    goto    ECC_1
    return

EventCountIncrement:
    movlw   EventCount
    movwf   FSR
    movlw   EventCountDigits
    movwf   EventCountIndex
ECI_1:
    incf    INDF,F
    movlw   '9'+1
    xorwf   INDF,W
    bnz     ECI_2
    movlw   10
    subwf   INDF,F
    incf    FSR,F
    decfsz  EventCountIndex,F
    goto    ECI_1
ECI_2:
    return

EventCountOutput:
    movlw   EventCountDigits
    movwf   EventCountIndex
    addlw   EventCount
    movwf   FSR
ECO_1:
    decf    FSR,F
    movf    INDF,W
    call    WriteDataXLCD
    decfsz  EventCountIndex,F
    goto    ECO_1
    return

MAIN_CODE code
; 
; This is the LCD test application.
;
; First open the LCD with a 4-bit 
; interface 5x7 character size
; and more than one line.
;
; Then send two lines to the LCD:
;   Line1: Ver 1.0 - Events
;   Line2: 0000000000000000
;
; Each HIGH to LOW transition on input pin
; RA4 (T0CLKI) will increment the count
; and display it on line 2 of the LCD.
;
main:
    movf    TMR0,W
    movwf   TMR0_Sample

    call    EventCountClear

    movlw   (FOUR_BIT&LINES_5X7)
    call    OpenXLCD

    movlw   LINE_ONE
    call    SetDDRamAddr

    movlw   LOW(LCD_message1)
    movwf   pszLCD_RomStr
    movlw   HIGH(LCD_message1)
    movwf   pszLCD_RomStr+1
    call    putrsXLCD

DisplayCount:
    movlw   LINE_TWO
    call    SetDDRamAddr
    call    EventCountOutput

PollForEvents:
    movf    TMR0_Sample,W
    subwf   TMR0,W
    skpnz
    goto    PollForEvents
    movwf   Events
    addwf   TMR0_Sample,F

CountEvents:
    call    EventCountIncrement
    decfsz  Events,F
    goto    CountEvents

    goto    DisplayCount

;
; LCD messages
;
LCD_message1:
    dt  "Ver 1.0 - Events",0

    END
