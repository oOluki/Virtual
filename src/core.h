#ifndef CORE_HEADER
#define CORE_HEADER

/*
 * QUICK MANUAL:
 * R0 the zero register, this register always holds the value 0, ALWAYS!. It is very usefull in some instructions as it serves as a dummy argument
 * RSP the register that holds the stack position
 * RIP the register that holds the instruction position
 * Rn stands for the nth argument which is a register in this case
 * Ln stands for the nth argument which is a literal in this case
 * E stands for the only argument which can be either a literal or a register
 * the .as_<type> suffix indicates that the value is to be read as the <type>, which also means that if the argument is a register it only needs sizeof(<type>) bytes.
 * the .<size> suffix indicates that only the first <size> less significant bits will be taken into account
 * if no suffix is provided the default is .64 for registers and .16 for literals (as literals have to be up to 16 bit, saved for the LOAD1 and LOAD2 arguments)
 * STACK refers to the stack (64 bit)
 * for valid instructions: <instruction_name> refers to INST_<instruction_name> in the enum OpCode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


typedef enum OpCode{

    // does nothing
    INST_NOP = 0,
    // halts the program with the status given in the argument
    // exit(E.as_int8)
    INST_HALT,

    // R1.8 = R2.8
    INST_MOV8,
    // R1.16 = R2.16
    INST_MOV16,
    // R1.32 = R2.32
    INST_MOV32,
    // R1.64 = R2.64
    INST_MOV,
    // if(R1.8 != 0x00) R2.64 = R3.64
    INST_MOVC,
    // R1 = L2
    INST_MOVV,
    // R1 = ~L2
    INST_MOVN,
    // R1.16 = L2
    INST_MOVV16,
    // pushes E on to the stack
    INST_PUSH,
    // pop the top of the stack into R1
    INST_POP,
    // R1 = STACK[RSP - L2.as_uint16]
    INST_STACK_GET,
    // STACK[RSP - L2.as_uint16] = R1
    INST_STACK_PUT,
    // R1.as_ptr = (uint8_t*) ((uint64_t*)(STACK_POINTER) + R2.as_uint64) + R3.as_uint64
    INST_GSP,
    // STACK[RSP++] = STATIC_POINTER + E.as_uint64
    INST_STATIC,
    // R1.8 = *(uint8_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ8,
    // R1.16 = *(uint16_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ16,
    // R1.32 = *(uint32_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ32,
    // R1 = *(uint64_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ,
    // reads R3.as_uint64 bytes from R2.as_ptr to R1.as_ptr
    // memcpy(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically
    INST_MREADS,
    // *(uint8_t*)(R1.as_ptr + R3.as_int64) = R2.8
    INST_WRITE8,
    // *(uint16_t*)(R1.as_ptr + R3.as_int64) = R2.16
    INST_WRITE16,
    // *(uint32_t*)(R1.as_ptr + R3.as_int64) = R2.32
    INST_WRITE32,
    // *(uint64_t*)(R1.as_ptr + R3.as_int64) = R2
    INST_WRITE,
    // sets R3.as_uint64 bytes of R1.as_ptr to R2.8
    // R1.as_ptr = memset(R1.as_ptr, R2.as_int8, R3.as_uint64) basically
    INST_MWRITES,
    // reads R3.as_uint64 bytes from R2.as_ptr to R1.as_ptr, accounting for overlaps
    // R1.as_ptr = memmove(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically
    INST_MMOVS,
    // compares R3.as_uint64 bytes of R2.as_ptr to R1.as_ptr and sets the truth value to R1.as_uint8
    // R1.as_uint8 = memmove(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically
    INST_MEMCMP,
    // R1 = !R2
    INST_NOT,
    // R1 = ~R2 | R3
    INST_NEG,
    // R1 = R2 & R3
    INST_AND,
    // R1 = ~(R2 & R3)
    INST_NAND,
    // R1 = R2 | R3
    INST_OR,
    // R1 = R2 ^ R3
    INST_XOR,
    // if(R2.as_int8 > 0) R1 = R2 <<  R3.as_uint8
    // else               R1 = R2 >> -R3.as_int8
    INST_BSHIFT,
    // RIP += E.as_int64 + 1
    INST_JMP,
    // if(R1.8 != 0x00) RIP += L2.as_int16
    // else             RIP += 1
    INST_JMPF,
    // if(R1.8 == 0x00) RIP += L2.as_int16
    // else             RIP += 1
    INST_JMPFN,
    // STACK[RSP++] = RIP.as_uint64 + 1
    // RIP += E.as_int64
    INST_CALL,
    // RIP = STACK[--RSP]
    INST_RET,
    // R1.as_uint8 = R2.as_uint8 + R3.as_uint8
    INST_ADD8,
    // R1.as_uint8 = R2.as_uint8 - R3.as_uint8
    INST_SUB8,
    // R1.as_uint8 = R2.as_uint8 * R3.as_uint8
    INST_MUL8,
    // R1.as_uint16 = R2.as_uint16 + R3.as_uint16
    INST_ADD16,
    // R1.as_uint16 = R2.as_uint16 - R3.as_uint16
    INST_SUB16,
    // R1.as_uint16 = R2.as_uint16 * R3.as_uint16
    INST_MUL16,
    // R1.as_uint32 = R2.as_uint32 + R3.as_uint32
    INST_ADD32,
    // R1.as_uint32 = R2.as_uint32 - R3.as_uint32
    INST_SUB32,
    // R1.as_uint32 = R2.as_uint32 * R3.as_uint32
    INST_MUL32,
    // R1.as_uint64 = R2.as_uint64 + R3.as_uint64
    INST_ADD,
    // R1.as_uint64 = R2.as_uint64 - R3.as_uint64
    INST_SUB,
    // R1.as_uint64 = R2.as_uint64 * R3.as_uint64
    INST_MUL,
    // R1.as_int64 = R2.as_int64 / R3.as_int64
    INST_DIVI,
    // R1.as_uint64 = R2.as_uint64 / R3.as_uint64
    INST_DIVU,
    // R1.as_float64 = R2.as_float64 + R3.as_float64
    INST_ADDF,
    // R1.as_float64 = R2.as_float64 - R3.as_float64
    INST_SUBF,
    // R1.as_float64 = R2.as_float64 * R3.as_float64
    INST_MULF,
    // R1.as_float64 = R2.as_float64 / R3.as_float64
    INST_DIVF,
    // R1.as_uint64 += L2.as_uint16
    INST_INC,
    // R1.as_uint64 -= L2.as_uint16
    INST_DEC,
    // R1.as_float64 += (double) L2.as_uint16
    INST_INCF,
    // R1.as_float64 -= (double) L2.as_uint16
    INST_DECF,
    // R1.as_uint64 = abs(R2.as_int64 - R3.as_int64)
    INST_ABS,
    // R1.as_float64 = abs(R2.as_float64 - R3.as_float64)
    INST_ABSF,
    // R1.as_uint8 = R2 != R3
    INST_NEQ,
    // R1.as_uint8 = R2 == R3
    INST_EQ,
    // R1.as_uint8 = R2.as_float64 == R3.as_float64
    INST_EQF,
    // R1.as_uint8 = R2.as_int64 > R3.as_int64
    INST_BIGI,
    // R1.as_uint8 = R2.as_uint64 > R3.as_uint64
    INST_BIGU,
    // R1.as_uint8 = R2.as_float64 > R3.as_float64
    INST_BIGF,
    // R1.as_uint8 = R2.as_int64 < R3.as_int64
    INST_SMLI,
    // R1.as_uint8 = R2.as_int64 < R3.as_int64
    INST_SMLU,
    // R1.as_uint8 = R2.as_float64 < R3.as_float64
    INST_SMLF,
    // R1.as_int64 = (int64_t) R2.as_uint64
    INST_CASTIU,
    // R1.as_int64 = (int64_t) R2.as_float64
    INST_CASTIF,
    // R1.as_uint64 = (uint64_t) R2.as_int64
    INST_CASTUI,
    // R1.as_uint64 = (uint64_t) R2.as_float64
    INST_CASTUF,
    // R1.as_float64 = (double) R2.as_int64
    INST_CASTFI,
    // R1.as_float64 = (double) R2.as_uint64
    INST_CASTFU,
    // R1.as_float32 = (float) R2.as_float64
    INST_CF3264,
    // R1.as_float64 = (double) R2.as_float32
    INST_CF6432,
    // R1.as_float64 = (double)(R2.as_int64) / (double)(R3.as_uint64)
    INST_FLOAT,
    // dumps R1.as_int8 character to stdout if R2.as_uint8 != 0 or stderr otherwise
    // and flushes the output stream if R3.as_uint8 != 0
    INST_DUMPCHAR,
    // reads a single character from stdin, or -1 if stdin is closed, to R1.as_int32
    // closes stdin if R2.as_uint8 != 0
    INST_GETCHAR,
    // executes the instruction given by R1.as_uint32
    INST_EXEC,
    // perfomrs a syscall identified by the value in E
    INST_SYS,
    // displays the R1, R2 and R3 register's values (ignores R0), for debugging purposes
    INST_DISREG,
    // for counting putposes
    INST_TOTAL_COUNT,
    // a dummy instruction that serves to hold immediate values, no use yet...
    INST_CONTAINER = 254,
    // this instruction is used for parsing purposes to signal an error while parsing a file, IT SHOULD NEVER APPEAR IN YOUR PROGRAM
    INST_ERROR = 255

} OpCode;

typedef uint32_t Inst;


enum Expects{
    EXPECT_ANY = 0,
    EXPECT_INST,
    EXPECT_OP_REG,
    EXPECT_OP_LIT,
    // LITERAL OR REGISTER
    EXPECT_OP_EITHER,
    EXPECT_OPTIONAL,
};

// R: REGISTER, L: LITERAL, E: EITHER LITERAL OR REGISTER ID
typedef enum OpProfile{

    OP_PROFILE_NONE = EXPECT_ANY,
    // instruction takes one register
    OP_PROFILE_R = EXPECT_OP_REG,
    // instruction takes one literal
    OP_PROFILE_L = EXPECT_OP_LIT,
    // instruction takes either a literal or a register
    OP_PROFILE_E = EXPECT_OP_EITHER,
    // instruction takes two registers
    OP_PROFILE_RR = (EXPECT_OP_REG << 8) | EXPECT_OP_REG,
    // instruction takes one register and a literal
    OP_PROFILE_RL = (EXPECT_OP_LIT << 8) | EXPECT_OP_REG,
    // instruction takes three registers
    OP_PROFILE_RRR = (EXPECT_OP_REG << 16) | (EXPECT_OP_REG << 8) | EXPECT_OP_REG,
    OP_PROFILE_RROR = ((EXPECT_OPTIONAL << 20) | (EXPECT_OP_REG << 16)) | (EXPECT_OP_REG << 8) | EXPECT_OP_REG

} OpProfile;

enum OpHint{

    HINT_REG = 0,
    HINT_LIT

};

enum ExeFlags{
    EXEFLAG_NONE = 0,
    EXEFLAG_LABELS_INCLUDED = 1 << 0
};

enum MajorRegisterId{
    MR0 = 0,

    MRA, MRB, MRC, MRD, MRE, MRF,
    MRG, MRH, MRI, MRJ, MRK, MRL,
    MRM, MRN, MRO, MRP, MRQ, MRR,
    MRS, MRT, MRU, MRV, MRW, MRX,
    MRY, MRZ,

    MRSP,
    MRIP,

    MRST = 31,

    // for counting purposes
    MR_COUNT
};

// valid registers id go from 0 to 255
enum RegisterId{
    // R0 = 0
    R0 = 0 ,

    // registers used inside the code

    RA = MRA * 8, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
    RB = MRB * 8, RB1, RB2, RB3, RB4, RB5, RB6, RB7,
    RC = MRC * 8, RC1, RC2, RC3, RC4, RC5, RC6, RC7,
    RD = MRD * 8, RD1, RD2, RD3, RD4, RD5, RD6, RD7,
    RE = MRE * 8, RE1, RE2, RE3, RE4, RE5, RE6, RE7,
    RF = MRF * 8, RF1, RF2, RF3, RF4, RF5, RF6, RF7,
    RG = MRG * 8, RG1, RG2, RG3, RG4, RG5, RG6, RG7,
    RH = MRH * 8, RH1, RH2, RH3, RH4, RH5, RH6, RH7,
    RI = MRI * 8, RI1, RI2, RI3, RI4, RI5, RI6, RI7,
    RJ = MRJ * 8, RJ1, RJ2, RJ3, RJ4, RJ5, RJ6, RJ7,
    RK = MRK * 8, RK1, RK2, RK3, RK4, RK5, RK6, RK7,
    RL = MRL * 8, RL1, RL2, RL3, RL4, RL5, RL6, RL7,
    RM = MRM * 8, RM1, RM2, RM3, RM4, RM5, RM6, RM7,
    RN = MRN * 8, RN1, RN2, RN3, RN4, RN5, RN6, RN7,
    RO = MRO * 8, RO1, RO2, RO3, RO4, RO5, RO6, RO7,
    RP = MRP * 8, RP1, RP2, RP3, RP4, RP5, RP6, RP7,
    RQ = MRQ * 8, RQ1, RQ2, RQ3, RQ4, RQ5, RQ6, RQ7,
    RR = MRR * 8, RR1, RR2, RR3, RR4, RR5, RR6, RR7,
    RS = MRS * 8, RS1, RS2, RS3, RS4, RS5, RS6, RS7,
    RT = MRT * 8, RT1, RT2, RT3, RT4, RT5, RT6, RT7,
    RU = MRU * 8, RU1, RU2, RU3, RU4, RU5, RU6, RU7,
    RV = MRV * 8, RV1, RV2, RV3, RV4, RV5, RV6, RV7,
    RW = MRW * 8, RW1, RW2, RW3, RW4, RW5, RW6, RW7,
    RX = MRX * 8, RX1, RX2, RX3, RX4, RX5, RX6, RX7,
    RY = MRY * 8, RY1, RY2, RY3, RY4, RY5, RY6, RY7,
    RZ = MRZ * 8, RZ1, RZ2, RZ3, RZ4, RZ5, RZ6, RZ7,

    // special registers

    // RSP holds the stack position
    RSP = MRSP * 8,
    // RIP holds the instruction position
    RIP = MRIP * 8,
    // holds the vpu status
    RST = MRST * 8,
    // holds error status
    RERROR = 255,

    // for counting purposes, not an actual register
    REGISTER_SPACE_SIZE
};

typedef union Register{
    
    int64_t  as_int64;
    int32_t  as_int32;
    int16_t  as_int16;
    int8_t   as_int8;

    uint64_t as_uint64;
    uint32_t as_uint32;
    uint16_t as_uint16;
    uint8_t  as_uint8;
    
    double   as_float64;
    float    as_float32;
    uint8_t* as_ptr;
} Register;

// Virtual Processing Unit
typedef struct VPU
{

    int         status;

    uint8_t*    static_memory;

    void*       system;

    Register*   registers;

    uint8_t*    register_space;

    Inst*       program;

    uint64_t*   stack;
    
} VPU;

#define GET_OP_HINT(INST) (INST >> 31)


#endif // =====================  END OF FILE CORE_HEADER ===========================
