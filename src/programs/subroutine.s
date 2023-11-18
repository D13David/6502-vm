
jmp start

STRLO       = $fb
STRHI       = $fc

; kernal routines
CHKOUT      = $FFC9
CHROUT      = $FFD2

msg         !byte "Hello, this is nice", $00

param !byte $00

start   lda     #3
        jsr     CHKOUT
        lda     #<msg
        sta     STRLO
        lda     #>msg
        sta     STRHI
        jsr     print
        rts

; ---------------------------------------------
; print subroutine
; ---------------------------------------------
print   ldy     #$ff
l0      iny
        lda     (STRLO), y
        jsr     CHROUT
        bne     l0
        rts