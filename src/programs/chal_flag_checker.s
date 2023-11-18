; =============================================================================
; INTIGRITI CTF 2023 - Reversing Challenge
; 
; Essentially a flag checker running in a 6502 inspired VM. Flag to be found 
; is: INTIGRITI{6502_VMs_R0ckss!}
; =============================================================================

; maximum input length
MAXLEN      = 70

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC

; kernal routines
CHKOUT      = $FFC9
CHRIN       = $FFCF
CHROUT      = $FFD2

jmp start

; some local string data used later

; SUCCESS... SHUTTING DOWN.
msg1        !byte $d4,$55,$d0,$d0,$51,$d4,$d4,$8b,$8b,$8b,$08,$d4,$12,$55,$15,$15,
                  $52,$93,$d1,$08,$11,$d3,$d5,$93,$8b,$82,$00
; ?SYNTAX  ERROR
msg2        !byte $cf,$d4,$56,$93,$15,$50,$16,$08,$08,$51,$94,$94,$d3,$94,$82,$00
; READY.
banner      !byte $94,$51,$50,$11,$56,$8b,$82,$00

; local variables
userinput   !byte 0*70

inputlen    !byte 0

; encoded flag
flag        !byte $94,$e5,$47,$97,$70,$20,$92,$42,$9c,$be,$69,$58,$0f,$2e,$fb,
                  $6a,$c4,$26,$e7,$36,$17,$23,$a0,$20,$2f,$0b,$cd,$00

; ---------------------------------------------
; print subroutine
; ---------------------------------------------
print   ldy     #$ff
l0      iny
        lda     (STRLO), y
        asl
        bcc     bit1
        ora     #1
bit1    asl
        bcc     chrout
        ora     #1
chrout  jsr     CHROUT
        bne     l0
        rts

; ---------------------------------------------
; program start
; ---------------------------------------------
start   ldx     #3
        jsr     CHKOUT

        lda     #<banner
        sta     STRLO
        lda     #>banner
        sta     STRHI
        jsr     print

; read user input
input   ldx     #0
        lda     #0
        sta     inputlen
l1      jsr     CHRIN

        ; check if user finished input by pressing enter
        cmp     #13
        beq     verify
        cmp     #10
        beq     verify

        ; check if user input length does not exeed max length
        pha
        lda     inputlen
        cmp     #MAXLEN
        pla
        beq     verify

        ; check character is in printable range 
        cmp     #$20
        bmi     l1
        cmp     #$7e
        bpl     l1

        sta     userinput, x

        inx
        inc     inputlen
        jmp     l1

; ---------------------------------------------
; verify user userinput
; ---------------------------------------------

; local variables
tmp     !byte 0
tmp1    !byte 0

verify  ldx     #$ff
        lda     #0
        sta     tmp1

        ; check if flag and input have same length
calclen inx                     
        lda     flag, x
        cmp     #00
        bne     calclen
        txa
        cmp     inputlen
        bne     fail

        ; loop over every character and compare input with decoded 
        ; flag character
        ldx     #00
loop0   lda     flag, x
        cmp     #00
        beq     check

        ; xor with key
        stx     tmp
        eor     tmp

        ; swap nibbles
        asl
        adc     #$80
        rol
        asl
        adc     #$80
        rol

        ; check current characer
        eor     userinput, x
        ora     tmp1
        sta     tmp1

        inx
        jmp     loop0

check   lda     tmp1
        bne     fail

; ---------------------------------------------
; print success message
; ---------------------------------------------
success lda     #<msg1
        sta     STRLO
        lda     #>msg1
        sta     STRHI
        jsr     print
        rts

; ---------------------------------------------
; print fail message
; ---------------------------------------------
fail    lda     #$0a
        jsr     CHROUT
        lda     #<msg2
        sta     STRLO
        lda     #>msg2
        sta     STRHI
        jsr     print
        jmp     start