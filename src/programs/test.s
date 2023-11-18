; hello world

LEN         = 20

; kernal routines
CHKOUT      = $FFC9
CHROUT      = $FFD2

jmp start

counter !byte 0
data    !byte $66,$6d,$63,$64,$7f,$71,$63,$74,$7c,$56,$7e,$6e,$7f,$79,$51,$7b,$75,$62,$66,$6e

start   lda     #3
        jsr     CHKOUT
        ldx     #0          ; initialize counter

loop    lda     data, x     ; load data+counter
        stx     counter
        eor     counter
        jsr     CHROUT      ; print char

        inx                 ; increase x pos
        cpx     #LEN        ; check string length
        bne     loop        ; jump back if not at end

        rts

