#ifndef BUILDER_H
#define BUILDER_H

#include "parser.h"
#include "../src/core.h"


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
    IIIOP_MUL = '*',
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
    IIATYPE_TUPLE,
    IIATYPE_III,

    IIATYPE_ERROR
};


typedef struct IntermediateInterpretationInstruction
{
    IIIOpCode   iiiop;
    int         arg[3];
} III;

typedef struct IntermediateInterpretationArgument
{
    int   type;
    union
    {
        char            c;
        unsigned char   uc;
        int             i;
        unsigned int    u;
        float           f;
        double          lf;
        void*           p;
        size_t          zu;
        uint64_t        u64;
        uint16_t        u16;
        char*           cstr;
        Str             str;
        TokenValue      tknv;
        struct 
        {
            int x;
            int y;
        } tuple;
        III             iii;
    } value;
} IIArg;

typedef struct Builder
{
    size_t scope_begin;
    size_t scope_end;
    DyArr  iii_index;
    DyArr  iias;

    uint64_t register_use_status;
    size_t   stack_size;
    DyArr    stack_bases;
    DyArr    assembly;

} Builder;



const char* get_iiiop_str(int iiiop);

int get_iiiop_from_str(const Str str);

// print Intermediate Interpretation Argument
// returns 0 on success or 1 on failure
int print_iia(FILE* output, const IIArg iia);

// print Intermediate Interpretation Instruction
// returns 0 on success or 1 on failure
int print_iii(FILE* output, const III iii);


int build_intermediate_interpretation();

int build_assembly();

extern Builder builder;


#endif // =====================  END OF FILE BUILDER_H ===========================