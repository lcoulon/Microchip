; -------------------------
; KEYPAD functions
; -------------------------
; 
#define KEYPAD_ASM
#include "main.inc"
#include "keypad.inc"
;
; How keypad is connected to PIC
;
; KP_1 <- RC0 Keypad row A    input to PIC
; KP_2 <- RC1 Keypad row B    input to PIC
; KP_3 -> RC4 Keypad column 1 output from PIC
; KP_4 -> RC5 Keypad column 2 output from PIC
; KP_5 -> RC6 Keypad column 3 output from PIC
; KP_6 -> RC7 Keypad column 4 output from PIC
; KP_7 <- RC2 Keypad row D    input to PIC
; KP_8 <- RC3 Keypad row C    input to PIC
;
KEYPAD_DATA     UDATA
kpSampleData    res     2
kpBounceData    res     2
kpStableData    res     2
kpChangedData   res     2
kpStableCount   res     1
kpKeyStatus     res     1

#define kpKeyStatusAvailable    kpKeyStatus,7
#define kpKeyStatusKeyDown      kpKeyStatus,6
#define kpKeyStatusKeyUp        kpKeyStatus,5
#define kpKeyStatusKeyCodeMask  0x0F

KEYPAD_CODE     CODE
;
; kpInit
;
kpInit:
    banksel kpKeyStatus
    clrf    kpKeyStatus
    clrf    kpStableCount
    clrf    kpStableData
    clrf    kpStableData+1
    clrf    kpBounceData
    clrf    kpBounceData+1

    return
;
; Count the number of ONE bits in the W register
; Uses kpChangedData for temporary storage
; Returns W = count of bits
;
CountBitsInW:
    movwf   kpChangedData       ; Use kpChangedData for temporary storage
    clrw
    btfsc   kpChangedData,7
    addlw   1
    btfsc   kpChangedData,6
    addlw   1
    btfsc   kpChangedData,5
    addlw   1
    btfsc   kpChangedData,4
    addlw   1
    btfsc   kpChangedData,3
    addlw   1
    btfsc   kpChangedData,2
    addlw   1
    btfsc   kpChangedData,1
    addlw   1
    btfsc   kpChangedData,0
    addlw   1
    return
;
; kpPoll
; Poll the keypad matrix.
; To be called once per millisecond
;
kpPoll:
;
; Read 4x4 keypad matrix and updates kpSambleData
;
    banksel TRISC
    movlw   B'01111111'
    movwf   TRISC
    banksel PORTC
    movwf   PORTC
    comf    PORTC,W
    andlw   0x0F
    banksel kpSampleData
    movwf   kpSampleData

    banksel TRISC
    movlw   B'10111111'
    movwf   TRISC
    banksel PORTC
    movwf   PORTC
    comf    PORTC,W
    andlw   0x0F
    banksel kpSampleData
    swapf   kpSampleData,F
    iorwf   kpSampleData,F

    banksel TRISC
    movlw   B'11011111'
    movwf   TRISC
    banksel PORTC
    movwf   PORTC
    comf    PORTC,W
    andlw   0x0F
    banksel kpSampleData
    movwf   kpSampleData+1

    banksel TRISC
    movlw   B'11101111'
    movwf   TRISC
    banksel PORTC
    movwf   PORTC
    comf    PORTC,W
    andlw   0x0F
    banksel kpSampleData
    swapf   kpSampleData+1,F
    iorwf   kpSampleData+1,F

    banksel TRISC
    movlw   B'11111111'
    movwf   TRISC
;
; Compares kpSampleData with kpBounceData
; Returns ZERO when kpSampleData matches kpBounceData
;
    banksel kpSampleData
    movf    kpSampleData,W
    xorwf   kpBounceData,W
    movwf   kpChangedData       ; Use kpChangedData for temporary storage
    xorwf   kpBounceData,F

    movf    kpSampleData+1,W
    xorwf   kpBounceData+1,W
    movwf   kpChangedData+1     ; Use kpChangedData+1 for temporary storage
    xorwf   kpBounceData+1,F

    movf    kpChangedData+1,W
    iorwf   kpChangedData,W
    skpnz                       ; Skip if key is bouncing
    goto    kpNotChanging
    clrf    kpStableCount       ; 
    return

kpNotChanging:
    incf    kpStableCount,W
    sublw   KP_DEBOUNCE_TIME
    skpz                        ; skip when bouncing done
    incf    kpStableCount,F
    skpz                        ; skip when bouncing done
    return
;
; Check if more that one key pressed
;
    movf    kpBounceData,W
    call    CountBitsInW
    movwf   kpChangedData+1
    movf    kpBounceData+1,W
    call    CountBitsInW
    addwf   kpChangedData+1,F
    movlw   -2
    addwf   kpChangedData+1,W
    skpnc
    return                      ; Return if more that one key pressed
;
; Test for key changed
;
    movf    kpBounceData,W
    xorwf   kpStableData,W
    xorwf   kpStableData,F
    movwf   kpChangedData

    movf    kpBounceData+1,W
    xorwf   kpStableData+1,W
    xorwf   kpStableData+1,F
    movwf   kpChangedData+1

    bankisel kpChangedData
    movlw   LOW(kpChangedData+1)
    movwf   FSR
    clrf    kpKeyStatus
    movf    INDF,F              ; Set STATUS ZERO if second 8-bit sample did not change
    skpz
    bsf     kpKeyStatus,4
    skpz
    goto    kpFindIndex

    decf    FSR,F
    clrf    kpKeyStatus
    movf    INDF,F              ; Set STATUS ZERO if second 8-bit sample did not change
    skpz
    bsf     kpKeyStatus,3
    skpz
    goto    kpFindIndex
    return

kpFindIndex:
    decf    kpKeyStatus,F
    btfsc   INDF,7
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,6
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,5
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,4
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,3
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,2
    goto    kpFoundKey

    decf    kpKeyStatus,F
    btfsc   INDF,1
    goto    kpFoundKey

    decf    kpKeyStatus,F
kpFoundKey:
    movf    INDF,W              ; Get change mask
    decf    FSR,F               ; Point for kpStableData
    decf    FSR,F
    andwf   INDF,W              ; W is 0 key released, W is non-zero for key pressed.
    skpz
    bsf     kpKeyStatusKeyDown
    skpnz
    bsf     kpKeyStatusKeyUp
    bsf     kpKeyStatusAvailable
    return
;
; kpGetStatus
; Returns status of new key pressed
; W = kpKeyStatus when new key pressed, STATUS NOT ZERO
; W = ZERO when no new key pressed, STATUS ZERO
;
kpGetStatus:
    banksel kpKeyStatus
    clrw
    btfsc   kpKeyStatusAvailable
    iorwf   kpKeyStatus,W
    bcf     kpKeyStatusAvailable
    return
;
; kpGetKey
; Return ASCII character of key
;
kpGetKey:
    banksel kpKeyStatus
    movf    kpKeyStatus,W
    andlw   (KP_KEY_EVENT_MASK | KP_KEY_DOWN_MASK | KP_KEY_UP_MASK)
    skpnz
    return

    movlw   HIGH(kpKeyTable)
    movwf   PCLATH
    movlw   kpKeyStatusKeyCodeMask
    andwf   kpKeyStatus,W
    addlw   LOW(kpKeyTable)
    btfsc   STATUS,C
    incf    PCLATH,F
    movwf   PCL
;
; kpKeyTable
; Table of LCD characters to keys
;
; x8-'1' xC-'2' x0-'3' x4-'R' ; Right arrow
; x9-'4' xD-'5' x1-'6' x5-'L' ; Left arrow
; xB-'7' xF-'8' x3-'9' x7-'U' ; Up arrow
; xA-'E' xE-'0' x2-'.' x6-'D' ; Down arrow
;
KEYPAD_CONST   code
kpKeyTable:
    dt      '3'
    dt      '6'
    dt      '.'
    dt      '9'
    dt      'R'  ; Right arrow
    dt      'L'  ; Left arrow
    dt      'D'  ; Down arrow
    dt      'U'  ; Up arrow
    dt      '1'
    dt      '4'
    dt      'E'  ; Enter
    dt      '7'
    dt      '2'
    dt      '5'
    dt      '0'
    dt      '8'                 
    end
