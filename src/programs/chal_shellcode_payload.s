
; kernal subroutines
SETLFS      = $FFBA
SETNAM      = $FFBD
LOAD        = $FFD5
CHROUT      = $FFD2

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC

NAMELEN     = 9

.org $c036

            jmp     start

filename        !byte "flag.txt",$00
buffer          !byte 0 * 42

start       lda     #NAMELEN
            ldx     #<filename
            ldy     #>filename
            jsr     SETNAM

            lda     #01
            ldx     $ba
            bne     skip
            ldx     #08             ; device 8
skip        ldy     #00             ; load to new address
            jsr     SETLFS

            ldx     #<buffer
            ldy     #>buffer
            lda     #00
            jsr     LOAD
            bcs     exit

            lda     #<buffer
            sta     STRLO
            lda     #>buffer
            sta     STRHI
            ldy     #$ff
l0          iny
            lda     (STRLO), y
chrout      jsr     CHROUT
            bne     l0

exit        rts