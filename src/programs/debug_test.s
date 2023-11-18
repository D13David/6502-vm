
DST = $FBEC

jmp start

payload !byte "%p %p %p %p", $0 

start   ldx     #$ff
loop    inx
        lda     payload, x
        sta     DST, x
        bne     loop

        ldx     #>DST
        ldy     #<DST
        int     #32 ; call debug

        rts