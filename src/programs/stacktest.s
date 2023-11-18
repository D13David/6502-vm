; stack test

; kernal routines
CHKOUT      = $FFC9
CHROUT      = $FFD2

        ldx     #3
        jsr     CHKOUT
        ldx     #91
fill    dex
        txa
        pha
        cmp     #65
        bne     fill

        ldx     #0
loop    pla
        jsr     CHROUT
        inx
        cpx     #26
        bne     loop

exit    rts
