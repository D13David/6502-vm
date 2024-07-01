#include "sml_vm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#if PLATFORM_WINDOWS
#include <windows.h>
#include <conio.h>

#define getch _getch
#endif

vm_context_t vm_context;

#define KERNAL_PROMPT_ADDR      0xFFFF

/// opcode handlers

/**
     0 immediate                op #$BB
     1 absolute                 op $LLHH
     2 absolute, x-indexed      op $LLHH, x
     3 absolute, y-indexed      op $LLHH, y
     4 implied                  op
     5 indirect                 op ($LLHH)
     6 X-indexed, indirect      op ($LL, x)
     7 indirect, Y-indexed      op ($LL), y
     8 relative                 op $BB
     9 zeropage                 op $LL
     A zeropage, X-indexed      op $LL, x
     B zeropage, Y-indexed      op $LL, y
*/

#define _ADDR_ABS       (fetch16())
#define _ADDR_ABSX      (fetch16() + vm_context.regs.X)
#define _ADDR_ABSY      (fetch16() + vm_context.regs.Y)
#define _ADDR_IND       (read_mem16(fetch16()))
#define _ADDR_INDX      (read_mem16(fetch8() + vm_context.regs.X))
#define _ADDR_INDY      (read_mem16(fetch8()) + vm_context.regs.Y)
#define _ADDR_ZPG       (fetch8())
#define _ADDR_ZPGX      ((fetch8() + vm_context.regs.X)&0xff)
#define _ADDR_ZPGY      ((fetch8() + vm_context.regs.Y)&0xff)

#define _ADDR_R_IMP     (vm_context.regs.AC)
#define _ADDR_R_IMM     (fetch8())
#define _ADDR_R_ABS     (read_mem8(_ADDR_ABS))
#define _ADDR_R_ABSX    (read_mem8(_ADDR_ABSX))
#define _ADDR_R_ABSY    (read_mem8(_ADDR_ABSY))
#define _ADDR_R_IND     (read_mem8(_ADDR_IND))
#define _ADDR_R_INDX    (read_mem8(_ADDR_INDX))
#define _ADDR_R_INDY    (read_mem8(_ADDR_INDY))
#define _ADDR_R_ZPG     (read_mem8(_ADDR_ZPG))
#define _ADDR_R_ZPGX    (read_mem8(_ADDR_ZPGX))
#define _ADDR_R_ZPGY    (read_mem8(_ADDR_ZPGY))

#define _ADDR_IDX_IMM   0
#define _ADDR_IDX_ABS   1
#define _ADDR_IDX_ABSX  2
#define _ADDR_IDX_ABSY  3
#define _ADDR_IDX_IND   4
#define _ADDR_IDX_INDX  5
#define _ADDR_IDX_INDY  6
#define _ADDR_IDX_ZPG   7
#define _ADDR_IDX_ZPGX  8
#define _ADDR_IDX_ZPGY  9
#define _ADDR_IDX_IMP   a

#define CONCAT0(a, b) a ## b
#define CONCAT(a, b) CONCAT0(a, b)

#define DEF_OP(name, mode) void CONCAT(__op_##name##_, _ADDR_IDX_##mode)() { __op_ ## name(_ADDR_R_ ## mode); }
#define DEF_OP_AC(name) void CONCAT(__op_##name##_, _ADDR_IDX_IMP)() { __op_ ## name ##_##ac(_ADDR_R_IMP); }
#define DEF_OP_ADDR(name, mode) void CONCAT(__op_##name##_, _ADDR_IDX_##mode)() { __op_ ## name(_ADDR_ ## mode); }

//////////////////////////////////////
// bitwise 
void __op_and(uint8_t value)
{
    vm_context.regs.AC = value & vm_context.regs.AC;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_asl_ac(uint8_t value)
{
    uint16_t tmp = value << 1;
    vm_context.regs.AC = tmp & 0xff;
    set_flag(FLAG_C, tmp & ~0xff);
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_asl(uint16_t addr)
{
    uint16_t tmp = read_mem8(addr) << 1;
    write_mem8(addr, tmp & 0xff);
    set_flag(FLAG_C, tmp & ~0xff);
    set_flag(FLAG_N, tmp & 0x80);
    set_flag(FLAG_Z, !(tmp & 0xff));
}

void __op_eor(uint8_t value)
{
    vm_context.regs.AC ^= value;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_lsr_ac(uint8_t value)
{
    vm_context.regs.AC = value >> 1;
    set_flag(FLAG_C, value & 1);
    set_flag(FLAG_N, 0);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_lsr(uint16_t addr)
{
    uint8_t value = read_mem8(addr);
    set_flag(FLAG_C, value & 1);
    value = value >> 1;
    write_mem8(addr, value);
    set_flag(FLAG_N, 0);
    set_flag(FLAG_Z, !value);
}

void __op_ora(uint8_t value)
{
    vm_context.regs.AC = value | vm_context.regs.AC;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_rol_ac(uint8_t value)
{
    uint16_t tmp = ((uint16_t)value << 1);
    if (vm_context.regs.SR & FLAG_C) {
        tmp |= 1;
    }
    vm_context.regs.AC = tmp & 0xff;
    set_flag(FLAG_C, tmp & ~0xff);
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_rol(uint16_t addr)
{
    assert(0 && "ROL not implemented");
}

void __op_ror_ac(uint8_t value)
{
    uint16_t tmp = (uint16_t)value;
    if (vm_context.regs.SR & FLAG_C) {
        tmp |= 0x100;
    }
    vm_context.regs.AC = (tmp >> 1) & 0xff;
    set_flag(FLAG_C, tmp & 1);
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_ror(uint16_t addr)
{
    uint16_t tmp = (uint16_t)read_mem8(addr);
    if (vm_context.regs.SR & FLAG_C) {
        tmp |= 0x100;
    }
    set_flag(FLAG_C, tmp & 1);
    tmp = (tmp >> 1) & 0xff;
    write_mem8(addr, (uint8_t)tmp);
    set_flag(FLAG_N, tmp & 0x80);
    set_flag(FLAG_Z, !tmp);
}

DEF_OP(and, IMM)
DEF_OP(and, ZPG)
DEF_OP(and, ZPGX)
DEF_OP(and, ABS)
DEF_OP(and, ABSX)
DEF_OP(and, ABSY)
DEF_OP(and, INDX)
DEF_OP(and, INDY)

DEF_OP_AC(asl)
DEF_OP_ADDR(asl, ZPG)
DEF_OP_ADDR(asl, ZPGX)
DEF_OP_ADDR(asl, ABS)
DEF_OP_ADDR(asl, ABSX)

DEF_OP(eor, IMM)
DEF_OP(eor, ZPG)
DEF_OP(eor, ZPGX)
DEF_OP(eor, ABS)
DEF_OP(eor, ABSX)
DEF_OP(eor, ABSY)
DEF_OP(eor, INDX)
DEF_OP(eor, INDY)

DEF_OP_AC(lsr)
DEF_OP_ADDR(lsr, ZPG)
DEF_OP_ADDR(lsr, ZPGX)
DEF_OP_ADDR(lsr, ABS)
DEF_OP_ADDR(lsr, ABSX)

DEF_OP(ora, IMM)
DEF_OP(ora, ZPG)
DEF_OP(ora, ZPGX)
DEF_OP(ora, ABS)
DEF_OP(ora, ABSX)
DEF_OP(ora, ABSY)
DEF_OP(ora, INDX)
DEF_OP(ora, INDY)

DEF_OP_AC(rol)
DEF_OP_ADDR(rol, ZPG)
DEF_OP_ADDR(rol, ZPGX)
DEF_OP_ADDR(rol, ABS)
DEF_OP_ADDR(rol, ABSX)

DEF_OP_AC(ror)
DEF_OP_ADDR(ror, ZPG)
DEF_OP_ADDR(ror, ZPGX)
DEF_OP_ADDR(ror, ABS)
DEF_OP_ADDR(ror, ABSX)

//////////////////////////////////////
// branch
void __op_bpl()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_N) == 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bmi()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_N) != 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bvc()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_V) == 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bvs()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_V) != 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bcc()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_C) == 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bcs()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_C) != 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_bne()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_Z) == 0) {
        vm_context.regs.PC += addr;
    }
}

void __op_beq()
{
    int addr = (int8_t)fetch8();
    if ((vm_context.regs.SR & FLAG_Z) != 0) {
        vm_context.regs.PC += addr;
    }
}

//////////////////////////////////////
// compare
void __op_cmp(uint8_t value)
{
    int16_t tmp = vm_context.regs.AC - value;
    set_flag(FLAG_N, tmp & 0x80);
    set_flag(FLAG_Z, !(tmp & 0xff));
    set_flag(FLAG_C, tmp & ~0xff);
}

void __op_bit(uint8_t value)
{
    uint8_t tmp = vm_context.regs.AC & value;
    set_flag(FLAG_N, value & 0x80);
    set_flag(FLAG_V, value & 0x40);
    set_flag(FLAG_Z, !tmp);
}

void __op_cpx(uint8_t value)
{
    int16_t tmp = vm_context.regs.X - value;
    set_flag(FLAG_N, tmp & 0x80);
    set_flag(FLAG_Z, !(tmp & 0xff));
    set_flag(FLAG_C, tmp & ~0xff);
}

void __op_cpy(uint8_t value)
{
    int16_t tmp = vm_context.regs.Y - value;
    set_flag(FLAG_N, tmp & 0x80);
    set_flag(FLAG_Z, !(tmp & 0xff));
    set_flag(FLAG_C, tmp & ~0xff);
}

DEF_OP(cmp, IMM)
DEF_OP(cmp, ZPG)
DEF_OP(cmp, ZPGX)
DEF_OP(cmp, ABS)
DEF_OP(cmp, ABSX)
DEF_OP(cmp, ABSY)
DEF_OP(cmp, INDX)
DEF_OP(cmp, INDY)

DEF_OP(bit, ZPG)
DEF_OP(bit, ABS)

DEF_OP(cpx, IMM)
DEF_OP(cpx, ZPG)
DEF_OP(cpx, ABS)

DEF_OP(cpy, IMM)
DEF_OP(cpy, ZPG)
DEF_OP(cpy, ABS)

//////////////////////////////////////
// flag
void __op_clc()
{
    set_flag(FLAG_C, 0);
}

void __op_cld()
{
    set_flag(FLAG_D, 0);
}

void __op_sec()
{
    set_flag(FLAG_C, 1);
}

void __op_sed()
{
    set_flag(FLAG_D, 1);
}

void __op_cli()
{
    set_flag(FLAG_I, 0);
}

void __op_clv()
{
    set_flag(FLAG_V, 0);
}

void __op_sei()
{
    set_flag(FLAG_I, 1);
}

//////////////////////////////////////
// jump
void __op_jmp()
{
    vm_context.regs.PC = fetch16();
}

void __op_jmp_1()
{
    assert(0 && "JMP (addr) not implemented");
}

void __op_rts()
{
    vm_context.regs.PC = stack_pop16();
}

void __op_jsr()
{
    stack_push16(vm_context.regs.PC + 2);
    vm_context.regs.PC = fetch16();
    // jump to kernal routines if in valid range
    if (vm_context.regs.PC >= 0xFF81 && vm_context.regs.PC <= 0xFFF3) 
    {
        kernal_routines_call();
        __op_rts();
    }
}

void __op_rti()
{
    vm_context.regs.SR = stack_pop8() & ~(FLAG_B | 0x20);
    vm_context.regs.PC = stack_pop16();
}

//////////////////////////////////////
// math
void __op_adc(uint8_t value)
{
    uint8_t addend = value;
    uint16_t tmp = vm_context.regs.AC + addend;
    if (vm_context.regs.SR & FLAG_C) {
        tmp++;
    }

    if (vm_context.regs.SR & FLAG_D)
    {
        printf("decimal mode not implemented");
        vm_panic();
    }
    else
    {
        set_flag(FLAG_V, is_overflow(vm_context.regs.AC, addend, tmp));
        set_flag(FLAG_N, has_sign(tmp));
        set_flag(FLAG_C, has_carry(tmp));
    }

    vm_context.regs.AC = tmp & 0xff;

    set_flag(FLAG_Z, is_zero(vm_context.regs.AC));
}

void __op_sbc(uint8_t value)
{
    uint8_t subtrahend = value;
    uint16_t tmp = vm_context.regs.AC - subtrahend;
    if (vm_context.regs.SR & FLAG_C) {
        tmp--;
    }
    if (vm_context.regs.SR & FLAG_D)
    {
        printf("decmimal mode not implemented");
        vm_panic();
    }
    else
    {
        set_flag(FLAG_V, is_overflow(vm_context.regs.AC, subtrahend, tmp));
        set_flag(FLAG_N, has_sign(tmp));
        set_flag(FLAG_C, has_carry(tmp));
    }

    vm_context.regs.AC = tmp & 0xff;

    set_flag(FLAG_Z, is_zero(vm_context.regs.AC));
}

DEF_OP(adc, IMM)
DEF_OP(adc, ZPG)
DEF_OP(adc, ZPGX)
DEF_OP(adc, ABS)
DEF_OP(adc, ABSX)
DEF_OP(adc, ABSY)
DEF_OP(adc, INDX)
DEF_OP(adc, INDY)

DEF_OP(sbc, IMM)
DEF_OP(sbc, ZPG)
DEF_OP(sbc, ZPGX)
DEF_OP(sbc, ABS)
DEF_OP(sbc, ABSX)
DEF_OP(sbc, ABSY)
DEF_OP(sbc, INDX)
DEF_OP(sbc, INDY)

//////////////////////////////////////
// memory

void __op_lda(uint8_t value)
{
    vm_context.regs.AC = value;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_sta(uint16_t addr)
{
    write_mem8(addr, vm_context.regs.AC);
}

void __op_ldx(uint8_t value)
{
    vm_context.regs.X = value;
    set_flag(FLAG_N, vm_context.regs.X & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.X);
}

void __op_stx(uint16_t addr)
{
    write_mem8(addr, vm_context.regs.X);
}

void __op_ldy(uint8_t value)
{
    vm_context.regs.Y = value;
    set_flag(FLAG_N, vm_context.regs.Y & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.Y);
}

void __op_sty(uint16_t addr)
{
    write_mem8(addr, vm_context.regs.Y);
}

void __op_dec(uint16_t addr)
{
    int16_t value = read_mem8(addr) - 1;
    write_mem8(addr, (uint8_t)value);
    set_flag(FLAG_N, value < 0);
    set_flag(FLAG_Z, !value);
}

void __op_inc(uint16_t addr)
{
    int16_t value = read_mem8(addr) + 1;
    write_mem8(addr, (uint8_t)value);
    set_flag(FLAG_N, value < 0);
    set_flag(FLAG_Z, value == 0);
}

DEF_OP(lda, IMM)
DEF_OP(lda, ZPG)
DEF_OP(lda, ZPGX)
DEF_OP(lda, ABS)
DEF_OP(lda, ABSX)
DEF_OP(lda, ABSY)
DEF_OP(lda, INDX)
DEF_OP(lda, INDY)


DEF_OP_ADDR(sta, ZPG)
DEF_OP_ADDR(sta, ZPGX)
DEF_OP_ADDR(sta, ABS)
DEF_OP_ADDR(sta, ABSX)
DEF_OP_ADDR(sta, ABSY)
DEF_OP_ADDR(sta, INDX)
DEF_OP_ADDR(sta, INDY)

DEF_OP(ldx, IMM)
DEF_OP(ldx, ZPG)
DEF_OP(ldx, ZPGY)
DEF_OP(ldx, ABS)
DEF_OP(ldx, ABSY)

DEF_OP_ADDR(stx, ZPG)
DEF_OP_ADDR(stx, ZPGY)
DEF_OP_ADDR(stx, ABS)

DEF_OP(ldy, IMM)
DEF_OP(ldy, ZPG)
DEF_OP(ldy, ZPGX)
DEF_OP(ldy, ABS)
DEF_OP(ldy, ABSX)

DEF_OP_ADDR(sty, ZPG)
DEF_OP_ADDR(sty, ZPGX)
DEF_OP_ADDR(sty, ABS)

DEF_OP_ADDR(dec, ZPG)
DEF_OP_ADDR(dec, ZPGX)
DEF_OP_ADDR(dec, ABS)
DEF_OP_ADDR(dec, ABSX)

DEF_OP_ADDR(inc, ZPG)
DEF_OP_ADDR(inc, ZPGX)
DEF_OP_ADDR(inc, ABS)
DEF_OP_ADDR(inc, ABSX)

//////////////////////////////////////
// register

void __op_tax()
{
    vm_context.regs.X = vm_context.regs.AC;
    set_flag(FLAG_N, vm_context.regs.X & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.X);
}

void __op_tay()
{
    vm_context.regs.Y = vm_context.regs.AC;
    set_flag(FLAG_N, vm_context.regs.Y & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.Y);
}

void __op_txa()
{
    vm_context.regs.AC = vm_context.regs.X;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_tya()
{
    vm_context.regs.AC = vm_context.regs.Y;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_inx()
{
    vm_context.regs.X += 1;
    set_flag(FLAG_N, vm_context.regs.X < 0);
    set_flag(FLAG_Z, vm_context.regs.X == 0);
}

void __op_iny()
{
    vm_context.regs.Y += 1;
    set_flag(FLAG_N, vm_context.regs.Y < 0);
    set_flag(FLAG_Z, vm_context.regs.Y == 0);
}

void __op_dex()
{
    vm_context.regs.X -= 1;
    set_flag(FLAG_N, vm_context.regs.X < 0);
    set_flag(FLAG_Z, vm_context.regs.X == 0);
}

void __op_dey()
{
    vm_context.regs.Y -= 1;
    set_flag(FLAG_N, vm_context.regs.X < 0);
    set_flag(FLAG_Z, vm_context.regs.X == 0);
}

//////////////////////////////////////
// stack

void __op_pha()
{
    stack_push8(vm_context.regs.AC);
}

void __op_pla()
{
    vm_context.regs.AC = stack_pop8();
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __op_php()
{
    stack_push8(vm_context.regs.SR);
}

void __op_plp()
{
    vm_context.regs.SR = stack_pop8();
}

void __op_tsx()
{
    vm_context.regs.X = vm_context.regs.SP;
    set_flag(FLAG_N, vm_context.regs.X & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.X);
}

void __op_txs()
{
    vm_context.regs.SP = vm_context.regs.X;
}

//////////////////////////////////////
// other

void __op_brk()
{
    assert(0 && "BRK not implemented");
}

void __op_nop()
{
}

void __op_int()
{
    int value = fetch8();
    switch (value)
    {
    case 31:
#if PLATFORM_WINDOWS
        DebugBreak();
#else
        __builtin_trap();
#endif
        break;
    default:
        vm_panic();
        break;
    }
}

void (*__op_codes[])() =
{
/*      00           01           02           03    04           05           06           07    08         09           0A           0B    0C           0D           0E           0F        */
/*00*/  &__op_brk  , &__op_ora_5, NULL       , NULL, NULL       , &__op_ora_7, &__op_asl_7, NULL, &__op_php, &__op_ora_0, &__op_asl_a, NULL, NULL       , &__op_ora_1, &__op_asl_1, NULL     ,
/*10*/  &__op_bpl  , &__op_ora_6, NULL       , NULL, NULL       , &__op_ora_8, &__op_asl_8, NULL, &__op_clc, &__op_ora_3, NULL       , NULL, NULL       , &__op_ora_2, &__op_asl_2, NULL     ,
/*20*/  &__op_jsr  , &__op_and_5, NULL       , NULL, &__op_bit_7, &__op_and_7, &__op_rol_7, NULL, &__op_plp, &__op_and_0, &__op_rol_a, NULL, &__op_bit_1, &__op_and_1, &__op_rol_1, NULL     ,
/*30*/  &__op_bmi  , &__op_and_6, NULL       , NULL, NULL       , &__op_and_8, &__op_rol_8, NULL, &__op_sec, &__op_and_3, NULL       , NULL, NULL       , &__op_and_2, &__op_rol_2, NULL     ,
/*40*/  &__op_rti  , &__op_eor_5, NULL       , NULL, NULL       , &__op_eor_7, &__op_lsr_7, NULL, &__op_pha, &__op_eor_0, &__op_lsr_a, NULL, &__op_jmp  , &__op_eor_1, &__op_lsr_1, NULL     ,
/*50*/  &__op_bvc  , &__op_eor_6, NULL       , NULL, NULL       , &__op_eor_8, &__op_lsr_8, NULL, &__op_cli, &__op_eor_3, NULL       , NULL, NULL       , &__op_eor_2, &__op_lsr_2, NULL     ,
/*60*/  &__op_rts  , &__op_adc_5, NULL       , NULL, NULL       , &__op_adc_7, &__op_ror_7, NULL, &__op_pla, &__op_adc_0, &__op_ror_a, NULL, &__op_jmp_1, &__op_adc_1, &__op_ror_1, NULL     ,
/*70*/  &__op_bvs  , &__op_adc_6, NULL       , NULL, NULL       , &__op_adc_8, &__op_ror_8, NULL, &__op_sei, &__op_adc_3, NULL       , NULL, NULL       , &__op_adc_2, &__op_ror_2, NULL     ,
/*80*/  NULL       , &__op_sta_5, NULL       , NULL, &__op_sty_7, &__op_sta_7, &__op_stx_7, NULL, &__op_dey, NULL       , &__op_txa  , NULL, &__op_sty_1, &__op_sta_1, &__op_stx_1, NULL     ,
/*90*/  &__op_bcc  , &__op_sta_6, NULL       , NULL, &__op_sty_8, &__op_sta_8, &__op_stx_9, NULL, &__op_tya, &__op_sta_3, &__op_txs  , NULL, NULL       , &__op_sta_2, NULL       , NULL     ,
/*A0*/  &__op_ldy_0, &__op_lda_5, &__op_ldx_0, NULL, &__op_ldy_7, &__op_lda_7, &__op_ldx_7, NULL, &__op_tay, &__op_lda_0, &__op_tax  , NULL, &__op_ldy_1, &__op_lda_1, &__op_ldx_1, NULL     ,
/*B0*/  &__op_bcs  , &__op_lda_6, NULL       , NULL, &__op_ldy_8, &__op_lda_8, &__op_ldx_9, NULL, &__op_clv, &__op_lda_3, &__op_tsx  , NULL, &__op_ldy_2, &__op_lda_2, &__op_ldx_3, NULL     ,
/*C0*/  &__op_cpy_0, &__op_cmp_5, NULL       , NULL, &__op_cpy_7, &__op_cmp_7, &__op_dec_7, NULL, &__op_iny, &__op_cmp_0, &__op_dex  , NULL, &__op_cpy_1, &__op_cmp_1, &__op_dec_1, NULL     ,
/*D0*/  &__op_bne  , &__op_cmp_6, NULL       , NULL, NULL       , &__op_cmp_8, &__op_dec_8, NULL, &__op_cld, &__op_cmp_3, NULL       , NULL, NULL       , &__op_cmp_2, &__op_dec_2, NULL     ,
/*E0*/  &__op_cpx_0, &__op_sbc_5, NULL       , NULL, &__op_cpx_7, &__op_sbc_7, &__op_inc_7, NULL, &__op_inx, &__op_sbc_0, &__op_nop  , NULL, &__op_cpx_1, &__op_sbc_1, &__op_inc_1, &__op_int,
/*F0*/  &__op_beq  , &__op_sbc_6, NULL       , NULL, NULL       , &__op_sbc_8, &__op_inc_8, NULL, &__op_sed, &__op_sbc_3, NULL       , NULL, NULL       , &__op_sbc_2, &__op_inc_2, NULL     ,
};                   

void vm_panic()
{
    printf("PANIC PANIC PANIC");
    exit(0);
}

void vm_init()
{
#ifndef PLATFORM_WINDOWS
    setbuf(stdin, NULL);
    setbuf(stderr, NULL);
    setbuf(stdout, NULL);
#endif //!PLATFORM_WINDOWS

    memset(&vm_context, 0, sizeof(vm_context));
    vm_context.regs.PC = 0;
    vm_context.regs.SP = 0xFF;

    kernal_init();
}

void vm_shutdown()
{
}

int vm_load(const uint8_t* data, long size)
{
    if (size > PROGRAM_SEG_SIZE) {
        return 0;
    }

    memcpy(&vm_context.memory[PROGRAM_SEG_OFFSET], data, size);

    stack_push16(KERNAL_PROMPT_ADDR);
    vm_context.regs.PC = PROGRAM_SEG_OFFSET;

    return 1;
}

int vm_cycle()
{
    uint8_t opcode = fetch8();
    void (*func)() = __op_codes[opcode];
    if (func == NULL) vm_panic();
    func();

    // program jumps out of execution
    if (vm_context.regs.PC == KERNAL_PROMPT_ADDR) {
        return 0;
    }

    return 1;
}

void vm_run()
{
    while (1) {
        if (vm_cycle() == 0) {
            break;
        }
    }
}