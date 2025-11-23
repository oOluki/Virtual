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
#include "virtual.h"


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

    RA = MRA * 8, RA1, RA2, RA3, RA4,
    RB = MRB * 8, RB1, RB2, RB3, RB4,
    RC = MRC * 8, RC1, RC2, RC3, RC4,
    RD = MRD * 8, RD1, RD2, RD3, RD4,
    RE = MRE * 8, RE1, RE2, RE3, RE4,
    RF = MRF * 8,

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


char get_digit_char(int i){
    switch (i)
    {
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';

    default: return '\0';
    }
}

char* get_reg_str(int reg, char* output){

    const int major_identifier = reg / 8;

    if(major_identifier >= MRA && major_identifier <= MRZ){
        output[0] = 'R';
        output[1] = 'A' + (major_identifier - MRA);
        output[2] = get_digit_char(reg - major_identifier * 8);
        output[3] = '\0';
        return output;
    }

    switch (major_identifier)
    {
    case MR0:
	output[0] = 'R';
	output[1] = '0';
	output[2] = get_digit_char(reg - R0);
	output[3] = '\0';
	return output;
    case MRSP:
        output[0] = 'R';
        output[1] = 'S';
        output[2] = 'P';
        output[3] = get_digit_char(reg - RSP);
        output[4] = '\0';
        return output;
    case MRIP:
        output[0] = 'R';
        output[1] = 'I';
        output[2] = 'P';
        output[3] = get_digit_char(reg - RIP);
        output[4] = '\0';
        return output;
    case MRST:
        output[0] = 'R';
        output[1] = 'S';
        output[2] = 'T';
        output[3] = get_digit_char(reg - RIP);
        output[4] = '\0';
        return output;
    
    default: return NULL;
    }
}


int print_inst_description(FILE* output, int inst){
    switch (inst)
    {
	case INST_NOP:
        fprintf(output, "NOP:\n");
        fprintf(output, "\tdoes nothing\n");
		return 0;
	case INST_HALT:
	    fprintf(output, "HALT:\n");
	    fprintf(output, "\thalts the program with the status given in the argument\n");
	    fprintf(output, "\texit(E.as_int8)\n");
		return 0;
	case INST_MOV8:
	    fprintf(output, "MOV8:\n");
	    fprintf(output, "\tR1.8 = R2.8\n");
		return 0;
	case INST_MOV16:
	    fprintf(output, "MOV16:\n");
	    fprintf(output, "\tR1.16 = R2.16\n");
		return 0;
	case INST_MOV32:
	    fprintf(output, "MOV32:\n");
	    fprintf(output, "\tR1.32 = R2.32\n");
		return 0;
	case INST_MOV:
	    fprintf(output, "MOV:\n");
	    fprintf(output, "\tR1.64 = R2.64\n");
		return 0;
	case INST_MOVC:
	    fprintf(output, "MOVC:\n");
	    fprintf(output, "\tif(R1.8 != 0x00) R2.64 = R3.64\n");
		return 0;
	case INST_MOVV:
	    fprintf(output, "MOVV:\n");
	    fprintf(output, "\tR1 = L2\n");
		return 0;
	case INST_MOVN:
	    fprintf(output, "MOVN:\n");
	    fprintf(output, "\tR1 = ~L2\n");
		return 0;
	case INST_MOVV16:
	    fprintf(output, "MOVV16:\n");
	    fprintf(output, "\tR1.16 = L2\n");
		return 0;
	case INST_PUSH:
	    fprintf(output, "PUSH:\n");
	    fprintf(output, "\tpushes E on to the stack\n");
		return 0;
	case INST_POP:
	    fprintf(output, "POP:\n");
	    fprintf(output, "\tpop the top of the stack into R1\n");
		return 0;
	case INST_STACK_GET:
	    fprintf(output, "STACK_GET:\n");
	    fprintf(output, "\tR1 = STACK[RSP - L2.as_uint16]\n");
		return 0;
	case INST_STACK_PUT:
	    fprintf(output, "STACK_PUT:\n");
	    fprintf(output, "\tSTACK[RSP - L2.as_uint16] = R1\n");
		return 0;
	case INST_GSP:
	    fprintf(output, "GSP:\n");
	    fprintf(output, "\tR1.as_ptr = (uint8_t*) ((uint64_t*)(STACK_POINTER) + R2.as_uint64) + R3.as_uint64\n");
		return 0;
	case INST_STATIC:
	    fprintf(output, "STATIC:\n");
	    fprintf(output, "\tSTACK[RSP++] = STATIC_POINTER + E.as_uint64\n");
		return 0;
	case INST_READ8:
	    fprintf(output, "READ8:\n");
	    fprintf(output, "\tR1.8 = *(uint8_t*)(R2.as_ptr + R3.as_uint64)\n");
		return 0;
	case INST_READ16:
	    fprintf(output, "READ16:\n");
	    fprintf(output, "\tR1.16 = *(uint16_t*)(R2.as_ptr + R3.as_uint64)\n");
		return 0;
	case INST_READ32:
	    fprintf(output, "READ32:\n");
	    fprintf(output, "\tR1.32 = *(uint32_t*)(R2.as_ptr + R3.as_uint64)\n");
		return 0;
	case INST_READ:
	    fprintf(output, "READ:\n");
	    fprintf(output, "\tR1 = *(uint64_t*)(R2.as_ptr + R3.as_uint64)\n");
		return 0;
	case INST_MREADS:
	    fprintf(output, "MREADS:\n");
	    fprintf(output, "\treads R3.as_uint64 bytes from R2.as_ptr to R1.as_ptr\n");
	    fprintf(output, "\tmemcpy(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically\n");
		return 0;
	case INST_WRITE8:
	    fprintf(output, "WRITE8:\n");
	    fprintf(output, "\t*(uint8_t*)(R1.as_ptr + R3.as_int64) = R2.8\n");
		return 0;
	case INST_WRITE16:
	    fprintf(output, "WRITE16:\n");
	    fprintf(output, "\t*(uint16_t*)(R1.as_ptr + R3.as_int64) = R2.16\n");
		return 0;
	case INST_WRITE32:
	    fprintf(output, "WRITE32:\n");
	    fprintf(output, "\t*(uint32_t*)(R1.as_ptr + R3.as_int64) = R2.32\n");
		return 0;
	case INST_WRITE:
	    fprintf(output, "WRITE:\n");
	    fprintf(output, "\t*(uint64_t*)(R1.as_ptr + R3.as_int64) = R2\n");
		return 0;
	case INST_MWRITES:
	    fprintf(output, "MWRITES:\n");
	    fprintf(output, "\tsets R3.as_uint64 bytes of R1.as_ptr to R2.8\n");
	    fprintf(output, "\tR1.as_ptr = memset(R1.as_ptr, R2.as_int8, R3.as_uint64) basically\n");
		return 0;
	case INST_MMOVS:
	    fprintf(output, "MMOVS:\n");
	    fprintf(output, "\treads R3.as_uint64 bytes from R2.as_ptr to R1.as_ptr, accounting for overlaps\n");
	    fprintf(output, "\tR1.as_ptr = memmove(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically\n");
		return 0;
	case INST_MEMCMP:
	    fprintf(output, "MEMCMP:\n");
	    fprintf(output, "\tcompares R3.as_uint64 bytes of R2.as_ptr to R1.as_ptr and sets the truth value to R1.as_uint8\n");
	    fprintf(output, "\tR1.as_uint8 = memmove(R1.as_ptr, R2.as_ptr, R3.as_uint64) basically\n");
		return 0;
	case INST_NOT:
	    fprintf(output, "NOT:\n");
	    fprintf(output, "\tR1 = !R2\n");
		return 0;
	case INST_NEG:
	    fprintf(output, "NEG:\n");
	    fprintf(output, "\tR1 = ~R2 | R3\n");
		return 0;
	case INST_AND:
	    fprintf(output, "AND:\n");
	    fprintf(output, "\tR1 = R2 & R3\n");
		return 0;
	case INST_NAND:
	    fprintf(output, "NAND:\n");
	    fprintf(output, "\tR1 = ~(R2 & R3)\n");
		return 0;
	case INST_OR:
	    fprintf(output, "OR:\n");
	    fprintf(output, "\tR1 = R2 | R3\n");
		return 0;
	case INST_XOR:
	    fprintf(output, "XOR:\n");
	    fprintf(output, "\tR1 = R2 ^ R3\n");
		return 0;
	case INST_BSHIFT:
	    fprintf(output, "BSHIFT:\n");
	    fprintf(output, "\tif(R2.as_int8 > 0) R1 = R2 <<  R3.as_uint8\n");
	    fprintf(output, "\telse               R1 = R2 >> -R3.as_int8\n");
		return 0;
	case INST_JMP:
	    fprintf(output, "JMP:\n");
	    fprintf(output, "\tRIP += E.as_int64 + 1\n");
		return 0;
	case INST_JMPF:
	    fprintf(output, "JMPF:\n");
	    fprintf(output, "\tif(R1.8 != 0x00) RIP += L2.as_int16\n");
	    fprintf(output, "\telse             RIP += 1\n");
		return 0;
	case INST_JMPFN:
	    fprintf(output, "JMPFN:\n");
	    fprintf(output, "\tif(R1.8 == 0x00) RIP += L2.as_int16\n");
	    fprintf(output, "\telse             RIP += 1\n");
		return 0;
	case INST_CALL:
	    fprintf(output, "CALL:\n");
	    fprintf(output, "\tSTACK[RSP++] = RIP.as_uint64 + 1\n");
	    fprintf(output, "\tRIP += E.as_int64\n");
		return 0;
	case INST_RET:
	    fprintf(output, "RET:\n");
	    fprintf(output, "\tRIP = STACK[--RSP]\n");
		return 0;
	case INST_ADD8:
	    fprintf(output, "ADD8:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_uint8 + R3.as_uint8\n");
		return 0;
	case INST_SUB8:
	    fprintf(output, "SUB8:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_uint8 - R3.as_uint8\n");
		return 0;
	case INST_MUL8:
	    fprintf(output, "MUL8:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_uint8 * R3.as_uint8\n");
		return 0;
	case INST_ADD16:
	    fprintf(output, "ADD16:\n");
	    fprintf(output, "\tR1.as_uint16 = R2.as_uint16 + R3.as_uint16\n");
		return 0;
	case INST_SUB16:
	    fprintf(output, "SUB16:\n");
	    fprintf(output, "\tR1.as_uint16 = R2.as_uint16 - R3.as_uint16\n");
		return 0;
	case INST_MUL16:
	    fprintf(output, "MUL16:\n");
	    fprintf(output, "\tR1.as_uint16 = R2.as_uint16 * R3.as_uint16\n");
		return 0;
	case INST_ADD32:
	    fprintf(output, "ADD32:\n");
	    fprintf(output, "\tR1.as_uint32 = R2.as_uint32 + R3.as_uint32\n");
		return 0;
	case INST_SUB32:
	    fprintf(output, "SUB32:\n");
	    fprintf(output, "\tR1.as_uint32 = R2.as_uint32 - R3.as_uint32\n");
		return 0;
	case INST_MUL32:
	    fprintf(output, "MUL32:\n");
	    fprintf(output, "\tR1.as_uint32 = R2.as_uint32 * R3.as_uint32\n");
		return 0;
	case INST_ADD:
	    fprintf(output, "ADD:\n");
	    fprintf(output, "\tR1.as_uint64 = R2.as_uint64 + R3.as_uint64\n");
		return 0;
	case INST_SUB:
	    fprintf(output, "SUB:\n");
	    fprintf(output, "\tR1.as_uint64 = R2.as_uint64 - R3.as_uint64\n");
		return 0;
	case INST_MUL:
	    fprintf(output, "MUL:\n");
	    fprintf(output, "\tR1.as_uint64 = R2.as_uint64 * R3.as_uint64\n");
		return 0;
	case INST_DIVI:
	    fprintf(output, "DIVI:\n");
	    fprintf(output, "\tR1.as_int64 = R2.as_int64 / R3.as_int64\n");
		return 0;
	case INST_DIVU:
	    fprintf(output, "DIVU:\n");
	    fprintf(output, "\tR1.as_uint64 = R2.as_uint64 / R3.as_uint64\n");
		return 0;
	case INST_ADDF:
	    fprintf(output, "ADDF:\n");
	    fprintf(output, "\tR1.as_float64 = R2.as_float64 + R3.as_float64\n");
		return 0;
	case INST_SUBF:
	    fprintf(output, "SUBF:\n");
	    fprintf(output, "\tR1.as_float64 = R2.as_float64 - R3.as_float64\n");
		return 0;
	case INST_MULF:
	    fprintf(output, "MULF:\n");
	    fprintf(output, "\tR1.as_float64 = R2.as_float64 * R3.as_float64\n");
		return 0;
	case INST_DIVF:
	    fprintf(output, "DIVF:\n");
	    fprintf(output, "\tR1.as_float64 = R2.as_float64 / R3.as_float64\n");
		return 0;
	case INST_INC:
	    fprintf(output, "INC:\n");
	    fprintf(output, "\tR1.as_uint64 += L2.as_uint16\n");
		return 0;
	case INST_DEC:
	    fprintf(output, "DEC:\n");
	    fprintf(output, "\tR1.as_uint64 -= L2.as_uint16\n");
		return 0;
	case INST_INCF:
	    fprintf(output, "INCF:\n");
	    fprintf(output, "\tR1.as_float64 += (double) L2.as_uint16\n");
		return 0;
	case INST_DECF:
	    fprintf(output, "DECF:\n");
	    fprintf(output, "\tR1.as_float64 -= (double) L2.as_uint16\n");
		return 0;
	case INST_ABS:
	    fprintf(output, "ABS:\n");
	    fprintf(output, "\tR1.as_uint64 = abs(R2.as_int64 - R3.as_int64)\n");
		return 0;
	case INST_ABSF:
	    fprintf(output, "ABSF:\n");
	    fprintf(output, "\tR1.as_float64 = abs(R2.as_float64 - R3.as_float64)\n");
		return 0;
	case INST_NEQ:
	    fprintf(output, "NEQ:\n");
	    fprintf(output, "\tR1.as_uint8 = R2 != R3\n");
		return 0;
	case INST_EQ:
	    fprintf(output, "EQ:\n");
	    fprintf(output, "\tR1.as_uint8 = R2 == R3\n");
		return 0;
	case INST_EQF:
	    fprintf(output, "EQF:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_float64 == R3.as_float64\n");
		return 0;
	case INST_BIGI:
	    fprintf(output, "BIGI:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_int64 > R3.as_int64\n");
		return 0;
	case INST_BIGU:
	    fprintf(output, "BIGU:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_uint64 > R3.as_uint64\n");
		return 0;
	case INST_BIGF:
	    fprintf(output, "BIGF:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_float64 > R3.as_float64\n");
		return 0;
	case INST_SMLI:
	    fprintf(output, "SMLI:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_int64 < R3.as_int64\n");
		return 0;
	case INST_SMLU:
	    fprintf(output, "SMLU:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_int64 < R3.as_int64\n");
		return 0;
	case INST_SMLF:
	    fprintf(output, "SMLF:\n");
	    fprintf(output, "\tR1.as_uint8 = R2.as_float64 < R3.as_float64\n");
		return 0;
	case INST_CASTIU:
	    fprintf(output, "CASTIU:\n");
	    fprintf(output, "\tR1.as_int64 = (int64_t) R2.as_uint64\n");
		return 0;
	case INST_CASTIF:
	    fprintf(output, "CASTIF:\n");
	    fprintf(output, "\tR1.as_int64 = (int64_t) R2.as_float64\n");
		return 0;
	case INST_CASTUI:
	    fprintf(output, "CASTUI:\n");
	    fprintf(output, "\tR1.as_uint64 = (uint64_t) R2.as_int64\n");
		return 0;
	case INST_CASTUF:
	    fprintf(output, "CASTUF:\n");
	    fprintf(output, "\tR1.as_uint64 = (uint64_t) R2.as_float64\n");
		return 0;
	case INST_CASTFI:
	    fprintf(output, "CASTFI:\n");
	    fprintf(output, "\tR1.as_float64 = (double) R2.as_int64\n");
		return 0;
	case INST_CASTFU:
	    fprintf(output, "CASTFU:\n");
	    fprintf(output, "\tR1.as_float64 = (double) R2.as_uint64\n");
		return 0;
	case INST_CF3264:
	    fprintf(output, "CF3264:\n");
	    fprintf(output, "\tR1.as_float32 = (float) R2.as_float64\n");
		return 0;
	case INST_CF6432:
	    fprintf(output, "CF6432:\n");
	    fprintf(output, "\tR1.as_float64 = (double) R2.as_float32\n");
		return 0;
	case INST_FLOAT:
	    fprintf(output, "FLOAT:\n");
	    fprintf(output, "\tR1.as_float64 = (double)(R2.as_int64) / (double)(R3.as_uint64)\n");
		return 0;
	case INST_DUMPCHAR:
	    fprintf(output, "DUMPCHAR:\n");
	    fprintf(output, "\tdumps R1.as_int8 character to stdout if R2.as_uint8 != 0 or stderr otherwise\n");
	    fprintf(output, "\tand flushes the output stream if R3.as_uint8 != 0\n");
		return 0;
	case INST_GETCHAR:
	    fprintf(output, "GETCHAR:\n");
	    fprintf(output, "\treads a single character from stdin, or -1 if stdin is closed, to R1.as_int32\n");
	    fprintf(output, "\tcloses stdin if R2.as_uint8 != 0\n");
		return 0;
	case INST_EXEC:
	    fprintf(output, "EXEC:\n");
	    fprintf(output, "\texecutes the instruction given by R1.as_uint32\n");
		return 0;
	case INST_SYS:
	    fprintf(output, "SYS:\n");
	    fprintf(output, "\tperfomrs a syscall identified by the value in E\n");
		return 0;
	case INST_DISREG:
	    fprintf(output, "DISREG:\n");
	    fprintf(output, "\tdisplays the R1, R2 and R3 register's values (ignores R0), for debugging purposes\n");
        return 0;
    default:
        fprintf(output, "NO INSTRUCTION FOR %i\n", inst);
        return 1;
    }
}


#endif // =====================  END OF FILE CORE_HEADER ===========================
