;   
; File: main.asm
; Target: PIC16F630, PIC16F676, PIC12F629, PIC12F675
; IDE: MPLABX v3.35
; Assembler: MPASM v5.62
;   
; Description:
;   Use TIMER1 to count a 10MHz input clock controlled by the gate input.
;   
;   There is more I should put in this description about how the serial
;   debug interface works. How to build for different targets. What
;   output means. Try reading the code.
;
; Notes:
;   It is not possible to use an external system clock and an
;   external TIMER1 clock.
;
;   Uses bit-banged UART on PGD/PGC pins so we can use the PICkit2
;   UART mode as debug serial interface.
;   
#ifdef __16F676
#include "p16F676.inc"
#define HAS_ADC
#endif
#ifdef __16F630
#include "p16F630.inc"
#endif
#ifdef __12F629
#include "p12F629.inc"
#define USE_16F_SYMBOLS
#endif
#ifdef __12F675
#include "p12F675.inc"
#define USE_16F_SYMBOLS
#define HAS_ADC
#endif
#ifdef USE_16F_SYMBOLS
#define PORTA GPIO
#define TRISA TRISIO
#define RAIE  GPIE
#define RAIF  GPIF
#define WPUA  WPU    
#define IOCA  IOC
#endif
    
    list    r=dec
    errorlevel -302
    
     __CONFIG _FOSC_INTRCIO & _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOREN_OFF 
     
#define FOSC (4000000)
#define FCYC (FOSC/4)
#define DELAY_COUNT (974)
;   
; Timer0 is clocked from the instruction cycle clock
;   
#define BAUD (19200)
#define T0_RELOAD (256-(FCYC/BAUD))
;
; Configure TIMER1 for:
;   TMR1GE  : Timer gate function is enabled, count when T1G is zero
;   T1CKPS  : Timer prescale is 1:1
;   T1OSCEN : Low Power oscillation amplifier is off
;   T1SYNC  : Not synchronized
;   TMR1CS  : Clock source is external input
;   TMR1ON  : Timer is off
;
#define TIMER1_CONTROL 0x46
    
   
RESET_VECTOR CODE  0x000
        pagesel start
        goto    start
    
ISR_DATA    UDATA_SHR
WREG_SAVE   res     1
STATUS_SAVE res     1
PCLATH_SAVE res     1
;
; Application wide status flags
App_Status  res     1
#define TickEvent App_Status,0
#define RX_DataAvailable App_Status,1
#define RX_FamingError App_Status,2
#define RX_OverrunError App_Status,3
;
; Serial data
TX_Data     res     1
RX_Data     res     1
UART_State  res     1

#define TX_START    (11)
#define RX_START    (16)
#define TXD_BIT     PORTA,0
#define RXD_BIT     PORTA,1
    
ISR_VECTOR  CODE    0x004
        movwf   WREG_SAVE       ; Save the current 
        movf    STATUS,W        ; execution context. 
        movwf   STATUS_SAVE     ; 
        movf    PCLATH,W        ; 
        movwf   PCLATH_SAVE     ; 
        
        clrf    STATUS          ; Set the ISR handler context to RAM 
        clrf    PCLATH          ; at bank zero and CODE in page zero.
;
; TIMER0 ISR
;   This is a bit banged half duplex UART
;
; Note:
;   The TIMER0 interrupt uses a PC relative
;   jump table to process the transmitter state.
;   This jump table must be within a 256 byte
;   program memory page.
;
ISR_TMR0:
        btfsc   INTCON,T0IE
        btfss   INTCON,T0IF
        goto    ISR_TMR0_EXIT

        bcf     INTCON,T0IF
        movlw   T0_RELOAD+3
        addwf   TMR0,F

        decfsz  UART_State,W      ; skip if transmitter is empty
        goto    UART_DoState
;
; TX empty
;
TX_State_0:
        bcf     INTCON,T0IE     ; Disable interrupt when TX empty
;
; Stop bit
;
TX_State_1:
        bsf     TXD_BIT
        goto    ISR_Exit

UART_DoState:
        andlw   0x0F            ; This prevents a crash but does not prevent
        movwf   UART_State      ; data errors when UART_state is corrupted.
        addwf   PCL,F
        goto    TX_State_0      ; TX empty
        goto    TX_State_1      ; stop
        goto    TX_State_2      ; data 7
        goto    TX_State_3      ; data 6
        goto    TX_State_4      ; data 5
        goto    TX_State_5      ; data 4
        goto    TX_State_6      ; data 3
        goto    TX_State_7      ; data 2
        goto    TX_State_8      ; data 1
        goto    TX_State_9      ; data 0
        goto    TX_State_10     ; start
        goto    TX_State_1      ; stop
        goto    TX_State_1      ; stop
        goto    TX_State_1      ; stop
        goto    RX_State_0      ; RX Stop bit
        goto    RX_State_1      ; RX Data bit
;
; Start bit
;
TX_State_10:
        bcf     TXD_BIT
        goto    ISR_Exit
;
; Data bits
;
TX_State_9:
TX_State_8:
TX_State_7:
TX_State_6:
TX_State_5:
TX_State_4:
TX_State_3:
TX_State_2:
        btfss   TX_Data,0
        bcf     TXD_BIT
        btfsc   TX_Data,0
        bsf     TXD_BIT
        rrf     TX_Data,F
        goto    ISR_Exit
;
; RX Stop bit
;
RX_State_0:
        btfss   RXD_BIT          ; Skip if stop bit present
        bsf     RX_FamingError
        btfsc   RX_DataAvailable ; Skip if data buffer available
        bsf     RX_OverrunError
        bsf     RX_DataAvailable ; Assert new data arrived
        bcf     INTCON,T0IE      ; Disable interrupt when RX complete
        goto    ISR_Exit
;
; RX Data bit
;
RX_State_1:
        btfsc   RXD_BIT         ; CARRY always zero when state starts
        setc                    ; Set CARRY if RX data is ZERO
        rrf     RX_Data,F       ; Shift in bit
        skpc                    ; CARRY is set when all 8 bits arrived
        incf    UART_State,F    ; Stay in RX data bit state until byte received
        goto    ISR_Exit
ISR_TMR0_EXIT:
;
; Interrupt On Change ISR
;   This is a bit banged UART receive start bit
ISR_IOC:
        btfsc   INTCON,RAIE
        btfss   INTCON,RAIF
        goto    ISR_IOC_EXIT

        movf    PORTA,W         ; Clear miss match
        bcf     INTCON,RAIF     ; Clear IOC assert
        btfsc   RXD_BIT         ; Skip if RXD is a Start Bit (LOW)
        goto    ISR_Exit

        movlw   T0_RELOAD+30    ; Add start bit ISR overhead
        movwf   TMR0
        bcf     INTCON,T0IF
        bsf     INTCON,T0IE

        movlw   0x80            ; Set to receive 8 bits of RX data
        movwf   RX_Data
        movlw   RX_START        ; Set UART to receive data bits state
        movwf   UART_State
        bcf     INTCON,RAIE     ; Disable RX start bit interrupt

        goto    ISR_Exit
ISR_IOC_EXIT:
    
ISR_Exit:
        movf    PCLATH_SAVE,W
        swapf   WREG_SAVE,F
        swapf   WREG_SAVE,W
        retfie
        movwf   PCLATH
        movf    STATUS_SAVE,W
        movwf   STATUS
;
; PutC
;   put a character out serial port
;
Putc:
        btfsc   INTCON,T0IE     ; skip when UART not busy
        goto    Putc
        bcf     INTCON,RAIE     ; cannot receive when sending

        banksel TX_Data
        movwf   TX_Data
        movlw   TX_START        ; select UART start send state
        movwf   UART_State

        banksel TMR0        ; Bank 0
        movlw   T0_RELOAD
        movwf   TMR0
        bcf     INTCON,T0IF
        bsf     INTCON,T0IE

        return
;
; Put HEX nibble
;
PutHexNibble:
        andlw   0x0F
        addlw   6
        btfsc   STATUS,DC
        addlw   'A'-'0'-10
        addlw   '0'-6
        goto    Putc
;
; Enable RX start bit detect
;
EnableStartBitDetect:
        movf    PORTA,W
        bcf     INTCON,RAIF
        bsf     INTCON,RAIE
        return
;
; Wait for UART transmit to complete
;
WaitForSend:
        btfsc   INTCON,T0IE     ; skip when UART not busy
        goto    WaitForSend
        return
;   
; Function that waits for at least one second
;    
DELAY_DATA  UDATA_SHR
D_counter   res 2

DELAY_CODE  code
DelayOneSecond:
        banksel D_counter
        movlw   HIGH(DELAY_COUNT)
        movwf   D_counter+1
        incf    D_counter+1,F
        movlw   LOW(DELAY_COUNT)
        movwf   D_counter
        clrw
DAL1:   addlw   -1
        skpz
        goto    DAL1
        decfsz  D_counter,F
        goto    DAL1
        decfsz  D_counter+1,F
        goto    DAL1
        
        return
;   
;   
;    
MAIN_CODE CODE
;
; Display TIMER1 value
;
DumpTimer1:
        banksel TMR1H
        swapf   TMR1H,W
        call    PutHexNibble
        movf    TMR1H,W
        call    PutHexNibble
        swapf   TMR1L,W
        call    PutHexNibble
        movf    TMR1L,W
        call    PutHexNibble

        movlw   0x0D
        call    Putc
        movlw   0x0A
        call    Putc

        call    WaitForSend
        return
;
; Pulse TIMER gate for 12 microseconds
;
PulseTimer1:
        movlw   TRISA
        movwf   FSR
        bcf     PORTA,4     ; make sure bit is low before we make it an output
        bcf     INDF,4      ; make T1G pin an output, and low
        goto    $+1
        goto    $+1
        goto    $+1
        goto    $+1
        goto    $+1
        nop
        bsf     PORTA,4     ; make T1G pin high
        bsf     INDF,4      ; make T1G pin an inut
        return
;
; Arm TIMER1 for counting when gate input is one
;
ArmTimer1:
        bcf     T1CON,TMR1ON
        clrf    TMR1H
        clrf    TMR1L
        bsf     T1CON,TMR1ON
        return
;
; Do a very simple command processor
;
ProcessCharacter:
        bcf     RX_DataAvailable

        movlw   'A'             ; see if it is a TIMER1 arm request
        xorwf   RX_Data,W
        skpnz
        call    ArmTimer1

        movlw   'G'             ; see if it is a TIMER1 gate pulse request
        xorwf   RX_Data,W
        skpnz
        call    PulseTimer1

        movlw   'T'             ; see if it is a TIMER1 request
        xorwf   RX_Data,W
        skpnz
        call    DumpTimer1

        call    EnableStartBitDetect
        return
;
; Initialize this PIC 
;
start:
        banksel INTCON      ; Bank 0
        clrf    INTCON      ; Disable interrupts
;
; Now we try to set the internal Oscillator Calibration
; If the call to fetch the OSCCAL value fails then we will go through
; reset but the NOT_POR bit will be one. When this happens we do not set
; the OSCCAL register. This will result in running with the oscillator
; in uncalibrated mode insead of looping through reset forever.
;
        banksel PCON
        btfsc   PCON,NOT_POR    ; skip if this is a POR
        goto    no_POR
        bsf     PCON,NOT_POR    ; clear status of Power On Reset
        call    0x3FF           ; retrieve factory calibration value
        movwf   OSCCAL          ; update register with factory cal value 
no_POR:
        bsf     PCON,NOT_BOD    ; clear status of Brown Out Detect
;
; Turn off comparators
;
        banksel CMCON
        movlw   0x07
        movwf   CMCON
;
; Wait for one second, this helps the ICSP when MCLR is used as an input
;
        call    DelayOneSecond
;
; Initialize output bits
;
        movlw   0x01
        movwf   PORTA       ; Set TXD bigh
#ifdef HAS_ADC
        banksel ANSEL       ; Bank 2
        clrf    ANSEL       ; Turn off analog inputs
#endif
        banksel OPTION_REG  ; Bank1
        movlw   0x5F        ; Set OPTION register
        movwf   OPTION_REG  ; TIMER0 clocks source is FCYC

        clrf    PIE1        ; Clear all peripheral interrupt enables.
        clrf    WPUA
        clrf    IOCA
        bsf     WPUA,1      ; Enable pull up on RXD
        bsf     WPUA,4      ; Enable pull up on T1G
        bsf     IOCA,1      ; Enable interrupt on change of RXD

;
; Set all GPIOs to outputs
;
        movlw   0x32        ; Set RXD(RA1), T1G(RA4), T1CLKI(RA5) as inputs
        movwf   TRISA
;
; Initialize serial output
;
        banksel UART_State
        bcf     INTCON,T0IE
        clrf    UART_State
;
; Initialize application status flags
;
        clrf    App_Status
;   
; Setup TIMER1
;   
; TIMER1 uses external low power crystal oscillator
; in asynchronous mode so it can wake us up from sleep.
;   
        banksel T1CON       ; Bank 0
        movlw   TIMER1_CONTROL
        movwf   T1CON
        bcf     PIR1,TMR1IF
        banksel PIE1        ; Bank 1
        bcf     PIE1,TMR1IE ; 
        banksel INTCON      ; Bank 0
        bsf     INTCON,PEIE ; 
    
        bsf     INTCON,GIE  ; Enable system interrupts
        call    DelayOneSecond
;
; Print the initial message: "Start<CR><LF>"
;
        movlw   'S'
        call    Putc
        movlw   't'
        call    Putc
        movlw   'a'
        call    Putc
        movlw   'r'
        call    Putc
        movlw   't'
        call    Putc
        movlw   0x0D
        call    Putc
        movlw   0x0A
        call    Putc
        call    WaitForSend
;
;
;
        call    ArmTimer1
        call    PulseTimer1
        call    DumpTimer1
;
; Start waiting for character
;
        call    EnableStartBitDetect
;
; This is the main process loop
;
ProcessLoop:
        btfsc   RX_DataAvailable
        call    ProcessCharacter
        
    
        goto    ProcessLoop
        
        end
