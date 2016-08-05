; 
; File: init.asm
; Target: PIC16F690
; IDE: MPLABX v3.35
; Compiler: MPASMWIN v5.68
;
; -------------------------
; Initialization 
; -------------------------
#define INIT_ASM
#include "main.inc"
;
; Configuration words.
; Define these in just one file.
;
        __CONFIG _FOSC_INTRCIO & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF
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
        goto    start                       ; begin PIC initialization

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

;------------------------------------------------------------------------
start:
    clrf    INTCON              ; Disable all interrupt sources
    clrf    TMR0
    banksel BANK1
    clrf    PIE1
    clrf    PIE2
    
    movlw   0x60
    movwf   OSCCON              ; Set internal oscillator to 4MHz
    
    movlw   b'11000001'         ; Pull-ups off, INT edge low to high, WDT prescale 1:1 
    movwf   OPTION_REG          ; TMR0 clock edge low to high, TMR0 clock = FCY, TMR0 prescale 1:4
            
    movlw   b'11111111'         ; 
    movwf   TRISA           
            
    movlw   b'01111111'         ; 
    movwf   TRISB           
            
    movlw   b'11111111'         ; 
    movwf   TRISC           
    
    banksel BANK2
    movlw   b'00000000'         ; Set all ADC inputs for digital I/O
    movwf   ANSEL           
    movlw   b'00000000'
    movwf   ANSELH
            
    banksel BANK0           
    clrf    ADCON0              ; Turn off ADC
    
    pagesel main
    goto    main

    END
