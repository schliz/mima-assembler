//
// Created by Christian Schliz on 16.11.21.
//

#ifndef MIMA_ASSEMBLER_OPCODES_H
#define MIMA_ASSEMBLER_OPCODES_H

// opcodes receiving a memory address
#define     LDC     0x0     // LoaD Constant
#define     LDV     0x1     // LoaD Value
#define     STV     0x2     // STore Value
#define     ADD     0x3     // ADD value
#define     AND     0x4     // logical AND
#define     OR      0x5     // logical OR
#define     XOR     0x6     // eXclusive OR
#define     EQL     0x7     // EQuaLs
#define     JMP     0x8     // JuMP
#define     JMN     0x9     // JuMp if Negative
#define     LDIV    0xA     // LoaD Indirect Value
#define     STIV    0xB     // STore Indirect Value
#define     JMS     0xC     // JumP Subroutine
#define     JIND    0xD     // Jump INDirect

// opcodes without arguments
#define     HALT    0xF0    // HALT
#define     NOT     0xF1    // logical NOT
#define     RAR     0xF2    // RotAte Right

#endif //MIMA_ASSEMBLER_OPCODES_H
