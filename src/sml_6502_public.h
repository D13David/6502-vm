#pragma once

/**
* $0000-$00FF   zero page
* $0100-$01FF   stack
* $0200-$03FF   reserved
* $0400-$07FF   video memory
* $0800-$9FFF   reserved
* $A000-$BFFF   reserved
* $C000-$CFFF   program storage
* $D000-$DFFF   reserved
* $E000-$FFFF   kernal rom
*/

#define MEMORY_SIZE         0xFFFF
#define STACK_OFFSET        0x0100
#define PROGRAM_SEG_OFFSET  0xC000

#define PROGRAM_SEG_SIZE    4096

#define FLAG_C              0x001 // carry
#define FLAG_Z              0x002 // zero
#define FLAG_I              0x004 // interrupt
#define FLAG_D              0x008 // decimal
#define FLAG_B              0x010 // break
#define FLAG_V              0x040 // overflow
#define FLAG_N              0x080 // negative

typedef struct
{
    struct
    {
        uint16_t PC;
        uint8_t  AC;
        uint8_t  X;
        uint8_t  Y;
        uint8_t  SR;
        uint8_t  SP;
    } regs;

    uint8_t memory[MEMORY_SIZE];

} vm_context_t;

extern vm_context_t vm_context;

void vm_panic();

#define REG_INC(ctx, r, c) ctx.regs.r += (c)

static inline uint8_t fetch8()
{
    if (vm_context.regs.PC + 1 >= (PROGRAM_SEG_OFFSET + PROGRAM_SEG_SIZE)) {
        vm_panic();
    }

    uint16_t pc = vm_context.regs.PC;
    REG_INC(vm_context, PC, 1);
    return vm_context.memory[pc];
}

static inline uint16_t fetch16()
{
    if (vm_context.regs.PC + 2 >= (PROGRAM_SEG_OFFSET + PROGRAM_SEG_SIZE)) {
        vm_panic();
    }

    uint16_t pc = vm_context.regs.PC;
    REG_INC(vm_context, PC, 2);
    return vm_context.memory[pc] | vm_context.memory[pc + 1] << 8;
}

static inline uint8_t read_mem8(uint16_t addr)
{
    return vm_context.memory[addr];
}

static inline uint16_t read_mem16(uint16_t addr)
{
    return vm_context.memory[addr] | vm_context.memory[addr + 1] << 8;
}

static inline void write_mem8(uint16_t addr, uint8_t value)
{
    vm_context.memory[addr] = value;
}

static inline void set_flag(int flag, int set)
{
    vm_context.regs.SR ^= ((-!!set) ^ vm_context.regs.SR) & flag;
}

static inline void stack_push8(uint8_t value)
{
    vm_context.memory[STACK_OFFSET + vm_context.regs.SP] = value;
    vm_context.regs.SP--;
}

static inline uint8_t stack_pop8()
{
    vm_context.regs.SP++;
    return vm_context.memory[STACK_OFFSET + vm_context.regs.SP];
}

static inline void stack_push16(uint16_t value)
{
    stack_push8((value >> 8) & 0xff);
    stack_push8(value & 0xff);
}

static inline uint16_t stack_pop16()
{
    uint16_t lo = stack_pop8();
    uint16_t hi = stack_pop8();
    return (hi << 8) | lo;
}

static inline int is_overflow(uint16_t a, uint16_t b, uint16_t sum)
{
    return ((sum ^ a) & (sum ^ b)) >> 15;
}

static inline int is_zero(uint16_t value)
{
    return !value;
}

static inline int has_sign(uint16_t value)
{
    return !!(value & 0x80);
}

static inline int has_carry(uint16_t value)
{
    return (value & ~0xff);
}