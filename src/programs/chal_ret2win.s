; =============================================================================
; INTIGRITI CTF 2023 - Pwn Challenge
; 
; Ret2Win but on 6502. Flag to be found is: INTIGRITI{d0_s0m3_r3tr0_pwn}
; =============================================================================

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC
STALO       = $FD
STAHI       = $FE

; kernal routines
CHKOUT      = $FFC9
CHRIN       = $FFCF
CHROUT      = $FFD2

MAXLEN      = 100

jmp start

flag        !byte "INTIGRITI{d0_s0m3_r3tr0_pwn}", $00
; flag      !byte "INTIGRITI{REDACTED_REDACTED}", $00 DEPLOY THIS IN HANDOUT
banner      !byte "Welcome! Something here...", $0a, $00
footer      !byte "Bye bye!", $00
hello       !byte "Hello ",$00

        
start   ldx     #3
        jsr     CHKOUT

        ; Print banner
        lda     #<banner
        sta     STRLO
        lda     #>banner
        sta     STRHI
        jsr     print

        ; Get user input
        jsr     input

        ; Print footer
        lda     #<footer
        sta     STRLO
        lda     #>footer
        sta     STRHI
        jsr     print

        ; Exit
        rts

; ---------------------------------------------
; read user input
; ---------------------------------------------
tmp         !byte 0
inputlen    !byte 0

input   clc
        ; store stack pointer and reserve some space on stack
        tsx
        stx     tmp
        txa
        sbc     #50
        tax
        dex
        txs

        ; store pointer to buffer start
        sta     STALO
        lda     #1
        sta     STAHI

        ; Read user input 
        ldy     #0
        lda     #0
        sta     inputlen
loop    jsr     CHRIN

        ; check if user input length does not exeed max length
        pha
        lda     inputlen
        cmp     #MAXLEN
        pla
        beq     ret
    
        sta     (STALO), y
        iny
        inc     inputlen

        ; check if user finished input by pressing enter
        cmp     #13             
        beq     ret
        cmp     #10
        beq     ret

        jmp     loop

ret     lda     #<hello
        sta     STRLO
        lda     #>hello
        sta     STRHI
        jsr     print
        lda     STALO
        sta     STRLO
        lda     STAHI
        sta     STRHI
        jsr     print
    
        ; Get return address back and return
        ldx      tmp
        txs
        rts

; ---------------------------------------------
; win subroutine
; ---------------------------------------------
win     lda     #<flag
        sta     STRLO
        lda     #>flag
        sta     STRHI
        jsr     print
        rts

; ---------------------------------------------
; print subroutine
; ---------------------------------------------
print   ldy     #$ff
l0      iny
        lda     (STRLO), y
chrout  jsr     CHROUT
        bne     l0
        rts