; -------------------------
; LED functions
; -------------------------
; 
#define LED_ASM
#include "main.inc"
#include "led.inc"
;
; How LED is connected to PIC
;
; RA4 -> LED
;
LED_DATA     UDATA

LED_CODE     CODE
;
; ldInit
;
ldInit:
    banksel BANK1
    bcf     TRISA,TRISA4
    banksel BANK0
    bcf     PORTA,RA4
    return
;
; ldOn
;
ldOn:
    banksel BANK0
    bsf     PORTA,RA4
    return
;
; ldOff
;
ldOff:
    banksel BANK0
    bcf     PORTA,RA4
    return
;
; ldToggle
;
ldToggle:
    banksel BANK0
    movlw   (1<<RA4)
    xorwf   PORTA,F
    return
    end
