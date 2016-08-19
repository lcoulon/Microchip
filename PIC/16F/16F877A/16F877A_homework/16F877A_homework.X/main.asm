;   
; File: main.asm
; Target: PIC16F877A
; IDE: MPLABX v3.35
; Assembler: MPASM v5.68
;   
; Description:
;
;   Do a program code in PIC ASSEMBLY LANGUAGE using MPLAB and after that do simulation using PROTEUS. 
;   The program that should I write is to :
;   Toggle the LEDs every half second in sequence: green, yellow, red, green,...
;   Count cycles to obtain timing.
;   Use 4 MHz crystal for 1 microsecond internal clock period.
;   USING timer0 INT.
;
; Notes:
;
;   Forum post: http://www.microchip.com/forums/FindPost/944088
;
;                         PIC16F877A
;                 +----------:_:----------+
;       VPP ->  1 : MCLR/VPP      PGD/RB7 : 40 <> PGD
;           <>  2 : RA0           PGC/RB6 : 39 <> PGC
;           <>  3 : RA1               RB5 : 38 <>
;           <>  4 : RA2               RB4 : 37 <>
;           <>  5 : RA3           PGM/RB3 : 36 <>
;           <>  6 : RA4               RB2 : 35 <>    
;           <>  7 : RA5               RB1 : 34 <>    
;           <>  8 : RE0               RB0 : 33 <>
;           <>  9 : RE1               VDD : 32 <- PWR
;           <> 10 : RE2               VSS : 31 <- GND
;       PWR -> 11 : VDD               RD7 : 30 <>
;       GND -> 12 : VSS               RD6 : 29 <>
;           -> 13 : OSC1              RD5 : 28 <>
;           <- 14 : OSC2              RD4 : 27 <>
;           <> 15 : RC0         RX/DT/RC7 : 26 <>
;           <> 16 : RC1         TX/CK/RC6 : 25 <>
;           <> 17 : RC2               RC5 : 24 <>
;           <> 18 : RC3               RC4 : 23 <>
; GREEN LED <> 19 : RD0               RD3 : 22 <>
;YELLOW LED <> 20 : RD1               RD2 : 21 <> RED LED
;                 +-----------------------:
;                          DIP-40
;
    list    r=dec,n=0,c=132
    errorlevel -302, -312
;
#include "p16F877A.inc"
    
     __CONFIG _FOSC_XT & _WDTE_OFF & _PWRTE_OFF & _BOREN_OFF & _LVP_OFF & _CPD_OFF & _WRT_OFF & _CP_OFF
     
#define FOSC (4000000)
#define FCYC (FOSC/4)
;
; Define macros to help
; with bank selection
;
#define BANK0  (h'000')
#define BANK1  (h'080')
#define BANK2  (h'100')
#define BANK3  (h'180')
;
; This RAM is used by the Interrupt Servoce Routine
; to save the context of the interrupted code.
INT_VAR     UDATA_SHR
w_temp      RES     1           ; variable used for context saving 
status_temp RES     1           ; variable used for context saving
pclath_temp RES     1           ; variable used for context saving
;
; Define LED port bits
;
#define RED_LED    PORTD,RD2
#define YELLOW_LED PORTD,RD1
#define GREEN_LED  PORTD,RD0
;
;**********************************************************************
RESET_VECTOR code 0x000         ; processor reset vector
    nop                         ; ICD2 needs this
    goto    start               ; begin PIC initialization
        
INT_VECTOR   code 0x004         ; interrupt vector location
        
INTERRUPT:      
    movwf   w_temp              ; save off current W register contents
    movf    STATUS,w            ; move status register into W register
    clrf    STATUS              ; force to bank zero
    movwf   status_temp         ; save off contents of STATUS register
    movf    PCLATH,W
    movwf   pclath_temp
    movlw   HIGH(INTERRUPT)
    movwf   PCLATH


; isr code can go here or be located as a called subroutine elsewhere


    movf    pclath_temp,W
    movwf   PCLATH
    movf    status_temp,w       ; retrieve copy of STATUS register
    movwf   STATUS              ; restore pre-isr STATUS register contents
    swapf   w_temp,f    
    swapf   w_temp,w            ; restore pre-isr W register contents
    retfie                      ; return from interrupt

;------------------------------------------------------------------------
start:
    clrf    INTCON              ; Disable all interrupt sources
    clrf    TMR0
    banksel BANK1
    clrf    PIE1
    clrf    PIE2
    
    movlw   b'11000000'         ; Pull-ups off, INT edge low to high, WDT prescale 1:1 
    movwf   OPTION_REG          ; TMR0 clock edge low to high, TMR0 clock = FCY, TMR0 prescale 1:2
            
    movlw   b'11111111'         ; 
    movwf   TRISA           
            
    movlw   b'01111111'         ; 
    movwf   TRISB           
            
    movlw   b'11111111'         ; 
    movwf   TRISC           
    
    movlw   b'11111000'         ; Make RD0, RD1, RD2 outputs for LEDs
    movwf   TRISD

    ; disable comparators
    movlw   b'00000111'
    movwf   CMCON
    
    ; Set all ADC inputs for digital I/O
    movlw   b'00000110'
    movwf   ADCON1
    
    banksel BANK0
    pagesel main
    goto    main
;
; Main application loop
;
MAIN_DATA udata 0x20

MAIN_CODE code
main:
;
; Setup TIMER0
;   TIMER0 clock source is FOSC/4 and the 1:2 prescale
;   this gives 8*256 (or 2048) oscillator clocks until 
;   the 8-bit timer asserts an overflow. Using a 4MHz
;   system oscillator this is 0.512 seconds.
;
    banksel TMR0
    clrf    TMR0                ; Reset 0.512 second timer
    bcf     INTCON,T0IF         ; Clear 0.512 second overflow flag
    ; Turn off all LEDs
    bcf     GREEN_LED
    bcf     YELLOW_LED
    bcf     RED_LED
;
; Loop here forever turning the LED on and off in sequence
;
ProcessLoop:
;
    bsf     GREEN_LED           ; Turn on Green LED
WaitOnGreen:    
    btfss   INTCON,T0IF 
    goto    WaitOnGreen         ; Wait for timer overflow
    bcf     INTCON,T0IF         ; Clear overflow flag
    bcf     GREEN_LED           ; Turn off Green LED
;   
    bsf     YELLOW_LED          ; Turn on Yellow LED
WaitOnYellow:   
    btfss   INTCON,T0IF 
    goto    WaitOnYellow        ; Wait for timer overflow
    bcf     INTCON,T0IF         ; Clear overflow flag
    bcf     YELLOW_LED          ; Turn off Yellow LED
;   
    bsf     RED_LED             ; Turn on Red LED
WaitOnRed:  
    btfss   INTCON,T0IF 
    goto    WaitOnRed           ; Wait for timer overflow
    bcf     INTCON,T0IF         ; Clear overflow flag
    bcf     RED_LED             ; Turn off Red LED
;
    goto    ProcessLoop
    
    end
    