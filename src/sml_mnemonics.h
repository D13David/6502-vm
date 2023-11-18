#define MM_NONE   0
#define MM_BRANCH 1
#define MM_JUMP   2

// implicite, immediate, abs, absx, absy, zero, zerox, zeroy, indx, indy, rel

// bitwise
DEFINE_MNENOMIC(AND, -1, 0x29, 0x2D, 0x3D, 0x39, 0x25, 0x35, -1, 0x21, 0x31, -1, MM_NONE)
DEFINE_MNENOMIC(ASL, 0x0A, -1, 0x0E, 0x1E, -1, 0x06, 0x16, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(EOR, -1, 0x49, 0x4D, 0x5D, 0x59, 0x45, 0x55, -1, 0x41, 0x51, -1, MM_NONE)
DEFINE_MNENOMIC(LSR, 0x4A, -1, 0x4E, 0x5E, -1, 0x46, 0x56, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(ORA, -1, 0x09, 0x0D, 0x1D, 0x19, 0x05, 0x15, -1, 0x01, 0x11, -1, MM_NONE)
DEFINE_MNENOMIC(ROL, 0x2A, -1, 0x2E, 0x3E, -1, 0x26, 0x36, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(ROR, 0x6A, -1, 0x6E, 0x7E, -1, 0x66, 0x77, -1, -1, -1, -1, MM_NONE)

// branch
DEFINE_MNENOMIC(BPL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x10, MM_BRANCH)
DEFINE_MNENOMIC(BMI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x30, MM_BRANCH)
DEFINE_MNENOMIC(BVC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x50, MM_BRANCH)
DEFINE_MNENOMIC(BVS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x70, MM_BRANCH)
DEFINE_MNENOMIC(BCC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x90, MM_BRANCH)
DEFINE_MNENOMIC(BCS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xB0, MM_BRANCH)
DEFINE_MNENOMIC(BNE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xD0, MM_BRANCH)
DEFINE_MNENOMIC(BEQ, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xF0, MM_BRANCH)

// compare
DEFINE_MNENOMIC(CMP, -1, 0xC9, 0xCD, 0xDD, 0xD9, 0xC5, 0xD5, -1, 0xC1, 0xD1, -1, MM_NONE)
DEFINE_MNENOMIC(BIT, -1, -1, 0x2C, -1, -1, 0x24, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(CPX, -1, 0xE0, 0xEC, -1, -1, 0xE4, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(CBY, -1, 0xC0, 0xCC, -1, -1, 0xC4, -1, -1, -1, -1, -1, MM_NONE)

// flag
DEFINE_MNENOMIC(CLC, 0x18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(CLD, 0xD8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(SEC, 0x38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(SED, 0xF8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(CLI, 0x58, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(CLV, 0xB8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(SEI, 0x78, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)

// jump
DEFINE_MNENOMIC(JMP, -1, -1, 0x4C, -1, -1, -1, -1, -1, -1, -1, -1, MM_JUMP)
DEFINE_MNENOMIC(RTS, 0x60, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_JUMP)
DEFINE_MNENOMIC(JSR, -1, -1, 0x20, -1, -1, -1, -1, -1, -1, -1, -1, MM_JUMP)
DEFINE_MNENOMIC(RTI, 0x40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_JUMP)

// math
DEFINE_MNENOMIC(ADC, -1, 0x69, 0x6D, 0x7D, 0x79, 0x65, 0x75, -1, 0x61, 0x71, -1, MM_NONE)
DEFINE_MNENOMIC(SBC, -1, 0xE9, 0xED, 0xFD, 0xF9, 0xE5, 0xF5, -1, 0xE1, 0xF1, -1, MM_NONE)

// memory
DEFINE_MNENOMIC(LDA, -1, 0xA9, 0xAD, 0xBD, 0xB9, 0xA5, 0xB5, -1, 0xA1, 0xB1, -1, MM_NONE)
DEFINE_MNENOMIC(STA, -1, -1, 0x8D, 0X9D, 0x99, 0x85, 0x95, -1, 0x81, 0x91, -1, MM_NONE)
DEFINE_MNENOMIC(LDX, -1, 0xA2, 0xAE, -1, 0xBE, 0xA6, -1, 0xB6, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(STX, -1, -1, 0x8E, -1, -1, 0x86, -1, 0x96, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(LDY, -1, 0xA0, 0xAC, 0xBC, -1, 0xA4, 0xB4, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(STY, -1, -1, 0x8C, -1, -1, 0x84, 0x94, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(DEC, -1, -1, 0xCE, 0xDE, -1, 0xC6, 0xD6, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(INC, -1, -1, 0xEE, 0xFE, -1, 0xE6, 0xF6, -1, -1, -1, -1, MM_NONE)

// register
DEFINE_MNENOMIC(TAX, 0xAA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(TAY, 0xA8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(TXA, 0x8A, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(TYA, 0x98, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(DEX, 0xCA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(DEY, 0x88, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(INX, 0xE8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(INY, 0xC8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)

// stack
DEFINE_MNENOMIC(PHA, 0x48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(PLA, 0x68, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(PHP, 0x08, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(PLP, 0x28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(TSX, 0xBA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(TXS, 0x9A, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)

// other
DEFINE_MNENOMIC(BRK, 0x00, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)
DEFINE_MNENOMIC(NOP, 0xEA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)

DEFINE_MNENOMIC(INT, -1, 0xEF, -1, -1, -1, -1, -1, -1, -1, -1, -1, MM_NONE)