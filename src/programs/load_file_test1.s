
; kernal subroutines
SETLFS      = $FFBA
SETNAM      = $FFBD
LOAD        = $FFD5
CHROUT      = $FFD2
CHRIN       = $FFCF
OPEN        = $FFC0
CHKIN       = $FFC6
CLOSE       = $FFC3
CLRCHN      = $FFCC
READST      = $FFB7

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC

NAMELEN     = 8

            jmp     start

filename        !byte "flag.txt"
load_success    !byte "File loaded...",$0a,$00
load_failed     !byte "Loading file failed...",$00

buffer          !byte 0 * 255

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
            lda     #$01
            ldx     $ba
            bne     skip
            ldx     #08
skip        ldy     #00
            jsr     SETLFS

            jsr     OPEN
            bcs     error

            ldx     #$01
            jsr     CHKIN

            lda     #<buffer
            sta     STRLO
            lda     #>buffer
            sta     STRHI

            ldy     #$00
loop        jsr     READST
            bne     eof
            jsr     CHRIN
            sta     (STRLO), y
            iny
            bne     skip2
            inc     STRHI
skip2       jmp     loop

eof         and     #$40
            beq     error

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

close       lda     #$01
            jsr     CLOSE
            jsr     CLRCHN
            rts

error       lda     #<load_failed
            sta     STRLO
            lda     #>load_failed
            sta     STRHI
            jsr     print
            jmp     close