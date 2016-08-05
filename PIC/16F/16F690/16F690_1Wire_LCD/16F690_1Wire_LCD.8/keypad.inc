; -------------------------
; KEYPAD functions
; -------------------------
;
#ifndef KEYPAD_INC
#define KEYPAD_INC
#ifdef  KEYPAD_ASM
#define ExtSymbol   global
#else
#define ExtSymbol   extern
#endif
;
; Declare the public symbols
;
    ExtSymbol   kpInit
    ExtSymbol   kpPoll
    ExtSymbol   kpGetStatus
    ExtSymbol   kpGetKey

; end of public symbols
#undefine ExtSymbol
;
; Define the KEYPAD constants
;
#define KP_DEBOUNCE_TIME    D'20'
#define KP_KEY_EVENT_MASK   0x80
#define KP_KEY_DOWN_MASK    0x40
#define KP_KEY_UP_MASK      0x20


#endif