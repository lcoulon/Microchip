; -------------------------
; OneWire network interface
; -------------------------
;
#ifndef ONEWIRE_INC
#define ONEWIRE_INC
#ifdef  ONEWIRE_ASM
#define ExtSymbol   global
#else
#define ExtSymbol   extern
#endif
;
; Declare the public symbols
;
    ExtSymbol   owWriteByte
    ExtSymbol   owReadByte
    ExtSymbol   owEnumFirst
    ExtSymbol   owEnumNext
    ExtSymbol   owReset,owWait18Cyc_Plus_W
    
; end of public symbols
#undefine ExtSymbol
;
; Define storage needs
#define OW_MAX_DEVICES  D'8'
#define OW_SIZE_SN D'8'
;
; Define the OneWire port pin
#define OW_PIN          PORTA,5
;
#define DELAY_240us     (d'240')
#define DELAY_205us     (d'205')
#define DELAY_70us      (d'70')
;
; OneWire connands
;
#define READ_ROM   0x33
#define SKIP_ROM   0xCC
#define MATCH_ROM  0x55
#define SEARCH_ROM 0xF0

#endif