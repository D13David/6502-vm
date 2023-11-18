
; kernal subroutines
SETLFS      = $FFBA
SETNAM      = $FFBD
LOAD        = $FFD5
CHROUT      = $FFD2

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC

NAMELEN     = 8

            jmp     start

filename        !byte "flag.txt"
load_success    !byte "File loaded...",$0a,$00
load_failed     !byte "Loading file failed...",$00

buffer          !byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

; ---------------------------------------------
; print subroutine
; ---------------------------------------------
print       ldy     #$ff
l0          iny
            lda     (STRLO), y
chrout      jsr     CHROUT
            bne     l0
            rts

            ; set filename for load operation
start       lda     #NAMELEN
            ldx     #<filename
            ldy     #>filename
            jsr     SETNAM

            ; setup input device for load
            lda     #01
            ldx     $ba
            bne     skip
            ldx     #08             ; device 8
skip        ldy     #00             ; load to new address
            jsr     SETLFS

            ; load file to target address
            ldx     #<buffer
            ldy     #>buffer
            lda     #00
            jsr     LOAD
            bcs     error

            ; print file content
            lda     #<load_success
            sta     STRLO
            lda     #>load_success
            sta     STRHI
            jsr     print

            lda     #<buffer
            sta     STRLO
            lda     #>buffer
            sta     STRHI
            jsr     print

            rts

error       lda     #<load_failed
            sta     STRLO
            lda     #>load_failed
            sta     STRHI
            jsr     print
            rts