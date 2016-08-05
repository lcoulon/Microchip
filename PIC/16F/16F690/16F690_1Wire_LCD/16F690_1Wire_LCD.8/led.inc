; -------------------------
; LED functions
; -------------------------
#ifndef LED_INC
#define LED_INC
#ifdef  LED_ASM
#define ExtSymbol   global
#else
#define ExtSymbol   extern
#endif
;
; Declare the public symbols
;
    ExtSymbol   ldInit
    ExtSymbol   ldOn
    ExtSymbol   ldOff
    ExtSymbol   ldToggle
    
; end of public symbols
#undefine ExtSymbol


#endif