#ifndef BUILDER_H
#define BUILDER_H

#include "parser.h"


enum VirtualInstOpCode{

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

};


// a<n> represent the nth argument
typedef enum IntermediateInterpretationInstructionOPCode {
    // does nothing
    IIIOP_NOP = 0,

    // declares a1 as variable of type a2
    IIIOP_VARDECL,
    // declares a1 as variable with ret type a2, and arguments of types given by the following arguments
    IIIOP_FUNCDECL,
    // opens scope
    IIIOP_OPENSCOPE,
    // closes scope
    IIIOP_CLOSESCOPE,

    // a1 = a2
    IIIOP_MOV = '=',
    // ...
    IIIOP_READ = 'd',
    // ...
    IIIOP_WRITE = 'w',
    // a1 = !a2
    IIIOP_NOT = '!',
    // a1 = ~a2
    IIIOP_NEG = '~',
    // a1 = a2 && a3
    IIIOP_AND = TKNTYPE_AA,
    // a1 = a2 & a3
    IIIOP_LAND = '&',
    // a1 = ~(a2 & a3)
    IIIOP_NAND = '@',
    // a1 = a2 || a3
    IIIOP_OR = TKNTYPE_OO,
    // a1 = a2 | a3
    IIIOP_LOR = '|',
    // a1 = a2 ^ a3
    IIIOP_XOR = '^',
    // a1 = a2 <<  a3
    IIIOP_LSHIFT = TKNTYPE_SL,
    // a1 = a2 >>  a3
    IIIOP_RSHIFT = TKNTYPE_SR,
    // a1 ~= a2
    IIIOP_NEGEQ = TKNTYPE_FLIPE,
    // a1 &&= a2
    IIIOP_ANDEQ = TKNTYPE_AAE,
    // a1 &= a2
    IIIOP_LANDEQ = TKNTYPE_AE,
    // a1 ||= a2
    IIIOP_OREQ = TKNTYPE_OOE,
    // a1 |= a2
    IIIOP_LOREQ = TKNTYPE_OE,
    // a1 ^= a2
    IIIOP_XOREQ = TKNTYPE_XE,
    // a1 <<= a2
    IIIOP_LSEQ = TKNTYPE_SLE,
    // a1 >>= a2
    IIIOP_RSEQ = TKNTYPE_SRE,
    // you know what this does...
    IIIOP_JMP = 'j',
    // if a1 -> JMP(a2)
    IIIOP_JMPF = 'J',
    // if not a1 -> JMP(a2)
    IIIOP_JMPFN = 'k',
    // a1 = a2(...)
    IIIOP_CALL = 'c',
    // a1 = a2 + a3
    IIIOP_ADD = '+',
    // a1 = a2 - a3
    IIIOP_SUB = '-',
    // a1 = a2 * a3
    IIIOP_MUL = 'X',
    // a1 = a2 / a3
    IIIOP_DIV = '/',
    // a1 += a2
    IIIOP_ADDEQ = TKNTYPE_PE,
    // a1 -= a2
    IIIOP_SUBEQ = TKNTYPE_ME,
    // a1 *= a2
    IIIOP_MULEQ = TKNTYPE_TE,
    // a1 /= a2
    IIIOP_DIVEQ = TKNTYPE_DE,
    // a1 = a2 % a3
    IIIOP_MOD = '%',
    // a1 += a2
    IIIOP_INC = TKNTYPE_PP,
    // a1 -= a2
    IIIOP_DEC = TKNTYPE_MM,
    // a1 = abs(a2)
    IIIOP_ABS = 'a',
    // a1 = a2 != a3
    IIIOP_NEQ = TKNTYPE_NE,
    // a1 = a2 == a3
    IIIOP_EQ = TKNTYPE_EE,
    // a1 = a2 > a3
    IIIOP_BIG = '>',
    // a1 = a2 < a3
    IIIOP_SML = '<',
    // a1 = (int) a2
    IIIOP_CASTI = 'i',
    // a1 = (float) a2
    IIIOP_CASTF = 'f',
    // a1 = (unsigned int) a2
    IIIOP_CASTU = 'u',
    // perfomrs a syscall
    IIIOP_SYS = 's',
    // ...
    IIIOP_RET = 'r',
    // EXIT(a1)
    IIIOP_EXIT = 'e',

    // this instruction is used for parsing purposes to signal an error while parsing a file, IT SHOULD NEVER APPEAR IN YOUR PROGRAM
    IIIOP_ERROR = 255
} IIIOpCode;

enum IntermediateInterpretationArgumentType{
    IIATYPE_NONE = 0,
    IIATYPE_ULIT = TKNTYPE_UINT,
    IIATYPE_ILIT = TKNTYPE_INT,
    IIATYPE_FLIT = TKNTYPE_FLOAT,
    IIATYPE_ZULIT,
    IIATYPE_SLIT = TKNTYPE_STR,
    IIATYPE_RAW = TKNTYPE_RAW,
    IIATYPE_SYM,
    IIATYPE_III,

    IIATYPE_ERROR
};


typedef struct IntermediateInterpretationArgument
{
    int   type;
    TokenValue value;
} IIArg;

typedef struct IntermediateInterpretationInstruction
{
    IIIOpCode   iiiop;
    int         arg[3];
} III;

typedef struct Builder
{
    size_t scope;
    DyArr  iiis;
    DyArr  iias;

} Builder;



const char* get_iiiop_str(int iiiop);

int get_iiiop_from_str(const Str str);

// print Intermediate Interpretation Argument
int print_iia(const IIArg iia);

// print Intermediate Interpretation Instruction
int print_iii(const III iii);


int interpret();


extern Builder builder;


#endif // =====================  END OF FILE BUILDER_H ===========================