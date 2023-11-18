; =============================================================================
; INTIGRITI CTF 2023 - Pwn Challenge
; 
; 6502 shellcode injection. 
; Flag to be found is: INTIGRITI{1nj3c71n6_5h3llc0d3_1n_7h3_805}}
; =============================================================================

; kernal subroutines
CHKOUT      = $FFC9
CHROUT      = $FFD2
CHRIN       = $FFCF

; store address to string for print subroutine
STRLO       = $FB
STRHI       = $FC

MAX_ENTRIES = 10
ENTRY_SIZE  = 58    ; 25 + 25 + 8
BUFFER_SIZE = 580   ; ENTRY_SIZE * BUFFER_SIZE

            jmp     start

num_entries !byte $00
buffer      !byte $00 * BUFFER_SIZE

header      !byte "=== Telephone Manager 1982 ===",$0a,$00
menu        !byte "1. add entry",$0a,
                  "2. list entries",$0a,
                  "3. exit",$0a,$00

first_name  !byte "enter first name: ",$00
last_name   !byte "enter last name: ",$00
phone       !byte "enter phone: ",$00
rec_added   !byte "new record added",$0a,$0a,$00

inv_input_e !byte "invalid input...",$0a,$0a,$00
space_empty !byte "your phonebook is full",$0a,$0a,$00
no_records  !byte "your phonebook has no records",$0a,$0a,$00

; ---------------------------------------------
; print subroutine
; ---------------------------------------------
print       stx     STRLO
            sty     STRHI
            ldy     #$ff
l0          iny
            lda     (STRLO), y
chrout      jsr     CHROUT
            bne     l0
            rts

; ---------------------------------------------
; multiply two 8 bit values
; ---------------------------------------------
mul_a       !byte 0
mul_b       !byte 0
mul_result  !byte 0

mul8        stx     mul_a
            sty     mul_b

            lda     #0
            ldx     #8
mul_l0      lsr     mul_b
            bcc     mul_l1
            clc
            adc     mul_a

mul_l1      ror
            ror     mul_result
            dex
            bne     mul_l0

            tay 
            lda     mul_result
            tax
            rts

; ---------------------------------------------
; add 8 bit to 16 bit value
; ---------------------------------------------
add16_value !byte 0
add16_lo    !byte 0
add16_hi    !byte 0

add16       stx     add16_lo
            sty     add16_hi
            sta     add16_value

            clc
            lda     add16_lo
            adc     add16_value
            tax
            bcc     add16_exit

            lda     add16_hi
            adc     #0
            tay
add16_exit  rts

; ---------------------------------------------
; main menu with user input
; ---------------------------------------------
key         !byte 0

main_menu   ldx     #<menu
            ldy     #>menu
            jsr     print

_menu_l0    jsr     CHRIN

            cmp     #13             
            beq     check_input
            cmp     #10
            beq     check_input

            sta     key
            jmp     _menu_l0

check_input lda     key
            cmp     #49
            beq     mnu0
            
            cmp     #50
            beq     mnu1

            cmp     #51
            beq     mnu2

            ldx     #<inv_input_e
            ldy     #>inv_input_e
            jsr     print
            jmp     main_menu

mnu0        jsr     add_entry
            jmp     main_menu

mnu1        jsr     list_all
            jmp     main_menu

mnu2        rts

; ---------------------------------------------
; add new entry
; ---------------------------------------------
offset_lo   !byte   0
offset_hi   !byte   0

add_entry   lda     num_entries
            cmp     #MAX_ENTRIES
            bmi     do_add

            ldx     #<space_empty
            ldy     #>space_empty
            jsr     print
            rts

do_add      ldx     num_entries
            ldy     #ENTRY_SIZE
            jsr     mul8
            stx     offset_lo
            sty     offset_hi

            lda     #<buffer
            adc     offset_lo
            sta     offset_lo
            lda     #>buffer
            adc     offset_hi
            sta     offset_hi
            
            ldx     #<first_name
            ldy     #>first_name
            jsr     print

            ldx     offset_lo
            ldy     offset_hi
            lda     #0
            jsr     add16
            jsr     user_input

            ldx     #<last_name
            ldy     #>last_name
            jsr     print

            ldx     offset_lo
            ldy     offset_hi
            lda     #25
            jsr     add16
            jsr     user_input

            ldx     #<phone
            ldy     #>phone
            jsr     print

            ldx     offset_lo
            ldy     offset_hi
            lda     #50
            jsr     add16
            jsr     user_input

            inc     num_entries

            ldx     #<rec_added
            ldy     #>rec_added
            jsr     print

            rts

; ---------------------------------------------
; list all entries
; ---------------------------------------------
count       !byte 0

list_all    lda     #<buffer
            sta     offset_lo
            lda     #>buffer
            sta     offset_hi

            lda     num_entries
            cmp     #0
            beq     list_err

            sta     count
list_l0     
            ldx     offset_lo
            ldy     offset_hi
            lda     #0
            jsr     add16
            jsr     print

            lda     #32
            jsr     CHROUT

            ldx     offset_lo
            ldy     offset_hi
            lda     #25
            jsr     add16
            jsr     print

            lda     #32
            jsr     CHROUT

            ldx     offset_lo
            ldy     offset_hi
            lda     #50
            jsr     add16
            jsr     print

            lda     #$0a
            jsr     CHROUT

            ldx     offset_lo
            ldy     offset_hi
            lda     #ENTRY_SIZE
            jsr     add16
            stx     offset_lo
            sty     offset_hi

            dec     count
            bne     list_l0

            lda     #$0a
            jsr     CHROUT
            jmp     list_exit

list_err    ldx     #<no_records
            ldy     #>no_records
            jsr     print

list_exit   rts

; ---------------------------------------------
; user input subroutine
; ---------------------------------------------
user_input  stx     STRLO
            sty     STRHI

            ldy     #0
usr_inp_l0  jsr     CHRIN
                
            cmp     #13
            beq     usr_inp_ext
            cmp     #10
            beq     usr_inp_ext

            sta     (STRLO),y
            iny
            bne     usr_inp_l0

            inc     STRHI
            ldy     #0

            jmp     usr_inp_l0

usr_inp_ext rts

; ---------------------------------------------
; main
; ---------------------------------------------
start       ldx     #3
            jsr     CHKOUT

            ldx     #<header
            ldy     #>header
            jsr     print

            jsr     main_menu

            rts