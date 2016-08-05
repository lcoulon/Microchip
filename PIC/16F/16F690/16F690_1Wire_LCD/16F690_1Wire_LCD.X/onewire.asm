; -------------------------
; OneWire network interface
; -------------------------
;
#define ONEWIRE_ASM
#include "main.inc"
#include "onewire.inc"
;
; Define data that must be visiable from all banks
;
ONEWIRE_DATA_SHR UDATA_SHR
owByte  res     1
;
; Define data that must be accessed with bank selects
;
ONEWIRE_DATA UDATA
;
; Reserve space for 1-Wire device serial numbers.
;
; Note: For a PIC16F690 it is impractical to 
;       have more than 8 devices as RAM bank
;       management becomes troublesome.
;
owIdList    res OW_MAX_DEVICES * OW_SIZE_SN

;
; Set OneWire port pin to tri-state
;
; Does not use W register
;
M_owHiZ:  macro
        banksel BANK1
        bsf     OW_PIN
        banksel BANK0
        endm
;
; Set OneWire port pin low, then drive on two instruction cycles later
;
; Does not use W register
;
M_owLow:  macro
        banksel BANK0
        bcf     OW_PIN
        banksel BANK1
        bcf     OW_PIN
        banksel BANK0
        endm
;
; Set CARRY to state of OneWire pin
;
; Does not use W register
;
M_owReadPin: macro
        banksel BANK1  
        bsf     OW_PIN      ; set OneWire pin for input
        banksel BANK0
        setc
        btfss   OW_PIN      ; Skip if input is high
        clrc
        endm
;
;
;
ONEWIRE_CODE code
;
; Cycle accurate delay from 18 cycles to 273 cycles.
;
owWait18Cyc_Plus_W:
    xorlw   HIGH(owWait18Cyc_Plus_Nops)
    movwf   PCLATH
    xorlw   HIGH(owWait18Cyc_Plus_Nops)
    xorwf   PCLATH,F
owWait18Cyc_Plus_Loop:
    addlw   -4
    btfsc   STATUS,C
    goto    owWait18Cyc_Plus_Loop
    xorlw   0xFF
    addlw   LOW(owWait18Cyc_Plus_Nops)
    btfsc   STATUS,C
    incf    PCLATH,F
    movwf   PCL
owWait18Cyc_Plus_Nops:
    nop
    nop
    nop
    return
;
; Waits for 5 microseconds with a system 
; oscillator of 4MHz, 1MHz instruction cycles.
;
; Does not use W, or STATUS flags
;
owWait5us:
    nop
owWait5usA:
    return
;
; Waits for 55,54 and 52 microseconds with a system 
; oscillator of 4MHz, 1MHz instruction cycles.
;
; Does not use W, or STATUS flags
;
owWait55us:
    nop
owWait54us:
    goto    owWait52us
owWait52us:
    call    owWait52usA
owWait52usA:
    call    owWait52usB
owWait52usB:
    nop
    call    owWait52usC
    call    owWait52usC
owWait52usC:
    return
;
; Function:         owReset
; PreCondition:     None
; Input:            None    
; Output:           W = 1 when device present, W = 0 when no device responds
; Overview:         Initialization sequence start with reset pulse.
;                   This code generates reset sequence as per the protocol.
;
owReset:
    M_owLow                         ; Drive OneWire pin low

    movlw   DELAY_240us-d'18'       ; Delay 480 microseconds
    call    owWait18Cyc_Plus_W
    movlw   DELAY_240us-d'18'
    call    owWait18Cyc_Plus_W
    
    M_owHiZ                         ; Drive OneWire pin high

    movlw   DELAY_70us-d'18'        ; Delay 70 microseconds
    call    owWait18Cyc_Plus_W
    
    M_owReadPin                     ; Sample for presence pulse from OneWire device
    bnc     owResetDeviceFound
    
    movlw   DELAY_205us-d'18'       ; Delay 410 microseconds
    call    owWait18Cyc_Plus_W
    movlw   DELAY_205us-d'18'
    call    owWait18Cyc_Plus_W
    retlw   0

owResetDeviceFound:
    movlw   DELAY_205us-d'18'       ; Delay 410 microseconds
    call    owWait18Cyc_Plus_W
    movlw   DELAY_205us-d'18'
    call    owWait18Cyc_Plus_W
    retlw   1
;
; Function:         owReadBit
; PreCondition:     None
; Input:            None
; Output:           Return CARRY set to the status of the OW PIN
; Overview:         This function used to read a single bit from the device.
;
owReadBit:
    movf    STATUS,W                ; save global interrupt enable in W
    bcf     INTCON,GIE              ; disable interrupts for a maximum of 60 microseconds

    M_owLow                         ; Drive OneWire pin low
    call    owWait5us
    M_owHiZ                         ; Drive OneWire pin high
    call    owWait5us
    M_owReadPin                     ; Set CARRY to OneWire pin state
    call    owWait55us

    andlw   (1<<GIE)
    iorwf   INTCON,F                ; restore global interrupt enable from CARRY
    return
;
; Function:         owWriteBit
; PreCondition:     None
; Input:            CARRY set write a OneWire one
;                   CARRY clear write a OneWire zero
; Output:           None
; Overview:         This function used to transmit a single bit to the device.
;
owWriteBit:
    movf    STATUS,W                ; save global interrupt enable in W
    bcf     INTCON,GIE              ; disable interrupts for a maximum of 60 microseconds
;
; Choose sending a one or zero
;
    bnc     owWriteBitZero

owWriteBitOne:
    M_owLow                         ; Drive OneWire pin low
    call    owWait5us
    M_owHiZ                         ; Drive OneWire pin high
    call    owWait55us
    
    andlw   (1<<GIE)
    iorwf   INTCON,F                ; restore global interrupt enable from CARRY
    return

owWriteBitZero:
    M_owLow                         ; Drive OneWire pin low
    call    owWait5us
    call    owWait55us
    M_owHiZ                         ; Drive OneWire pin high
    
    andlw   (1<<GIE)
    iorwf   INTCON,F                ; restore global interrupt enable from CARRY
    return
;
; Function:         owWriteByte
; PreCondition:     None
; Input:            W = byte to send to 1-wire slave device
; Output:           None
; Overview:         This function used to transmit a complete byte to slave device.
;
owWriteByte:
    movwf   owByte                  ; save byte to write
    setc                            ; Use shift to count bits
    rrf     owByte,F                ; CARRY is next bit to send
owWriteByte_NextBit:
    call    owWriteBit
    clrc                            ; shift in zeros
    rrf     owByte,F                ; CARRY is next bit to send
    movf    owByte,F
    bnz     owWriteByte_NextBit     ; loop until all 8-bits sent
    return
;
; Function:         owReadByte
; PreCondition:     None
; Input:            None
; Output:           W = the read byte from slave device
; Overview:         This function used to read a complete byte from the slave device.
;
owReadByte:
    clrf    owByte                  ; use shift to count bits
    bsf     owByte,7
owReadByte_NextBit:
    call    owReadBit               ; CARRY = input bit
    rrf     owByte,F                ; accumulate the byte LSB first
    bnc     owReadByte_NextBit      ; loop until all 8-bits sent
    movf    owByte,W
    return
;
; 
;
owEnumFirst:
    return
;
; 
;
owEnumNext:
    return
;
; 
;
owCRC8:
    return
;
; 
;
owCRC16:
    return

    end
