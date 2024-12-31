#ifndef CORE_HEADER
#define CORE_HEADER

#include "stdint.h"

typedef enum ExeFlags{

    EXE_DEFAULT = 0,

} ExeFlags;


typedef enum OpCode{

    // does nothing
    INST_NOP = 0,
    INST_HALT,

    // moves an 8 bit value from a register into another register
    INST_MOV8,
    // moves a 16 bit value from a register into another register
    INST_MOV16,
    // moves a 32 bit value from a register into another register
    INST_MOV32,
    // moves a 64 bit value from a register into another register
    INST_MOV,
    // moves a 64 bit value directly into a register
    INST_MOVV,
    // pushes a 64 bit value to the stack from a register
    INST_PUSH,
    // pushes a 64 bit value directly to the stack
    INST_PUSHV,
    // pops a 64 bit value from the stack into a register
    INST_POP,
    // reads a 64 bit value from the stack into a register
    INST_GET,
    // writes a 64 bit value from a register to the stack
    INST_WRITE,
    // writes the pointer to the beginning of the stack into a register
    INST_GSP,
    INST_STATIC,
    INST_READ8,
    INST_READ16,
    INST_READ32,
    INST_READ,
    INST_SET8,
    INST_SET16,
    INST_SET32,
    INST_SET,

    INST_TEST,
    INST_NOT,
    INST_NEG,
    INST_AND,
    INST_NAND,
    INST_OR,
    INST_XOR,
    INST_BSHIFT,

    INST_JMP,
    INST_JMPIF,
    INST_JMPIFN,
    INST_CALL,
    INST_RET,

    INST_ADD8,
    INST_SUB8,
    INST_MUL8,
    INST_ADD16,
    INST_SUB16,
    INST_MUL16,
    INST_ADD32,
    INST_SUB32,
    INST_MUL32,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIVI,
    INST_DIVU,
    INST_ADDF,
    INST_SUBF,
    INST_MULF,
    INST_DIVF,

    INST_EQI,
    INST_EQU,
    INST_EQF,

    INST_BIGI,
    INST_BIGU,
    INST_BIGF,

    INST_SMLI,
    INST_SMLU,
    INST_SMLF,

    INST_CASTIU,
    INST_CASTIF,

    INST_CASTUI,
    INST_CASTUF,

    INST_CASTFI,
    INST_CASTFU,
    INST_CF3264,
    INST_CF6432,

    INST_MEMSET,
    INST_MEMCPY,
    INST_MEMMOV,
    INST_MEMCMP,
    INST_MALLOC,
    INST_FREE,

    INST_FOPEN,
    INST_FCLOSE,
    INST_PUTC,
    INST_GETC,

    INST_SYS = 253,

    INST_DISREG = 254,

    INST_ERROR = 255

} OpCode;

typedef uint32_t Inst;


enum Expects{
    EXPECT_ANY = 0,
    EXPECT_INST,
    EXPECT_OP_REG,
    EXPECT_OP_LIT,
    EXPECT_OP_STR,
    EXPECT_IDENTIFIER,
};

// R: REGISTER, L: LITERAL, O: OPTIONAL_LITERAL
typedef enum OpProfile{

    OP_PROFILE_NONE = EXPECT_ANY,
    // instruction takes one register
    OP_PROFILE_R = EXPECT_OP_REG,
    // instruction takes one literal
    OP_PROFILE_L = EXPECT_OP_LIT,
    // instruction takes two registers
    OP_PROFILE_RR = (EXPECT_OP_REG << 8) | EXPECT_OP_REG,
    // instruction takes one register and a literal
    OP_PROFILE_RL = (EXPECT_OP_LIT << 8) | EXPECT_OP_REG,
    // instruction takes three registers
    OP_PROFILE_RRR = (EXPECT_OP_REG << 16) | (EXPECT_OP_REG << 8) | EXPECT_OP_REG,

} OpProfile;


enum RegisterId{

    RA = 0 , RA1, RA2, RA3, RA4,
    RB = 8 , RB1, RB2, RB3, RB4,
    RC = 16, RC1, RC2, RC3, RC4,
    RD = 24, RD1, RD2, RD3, RD4,
    RE = 32, RE1, RE2, RE3, RE4,
    RF = 40,

    RSP = 48,
    RIP = 56,
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
    void*    as_ptr;
} Register;

typedef struct VPU
{
    uint8_t* static_memory;

    uint8_t* internal_data;

    Register registers[8];

    uint8_t* register_space;

    uint64_t* stack;
    
} VPU;


static inline int is_little_endian(){ return (*(unsigned short *)"\x01\x00" == 0x01); }

static inline uint32_t swap32(uint32_t x){
    return (
        ((x & 0X000000FF) << 24) |
        ((x & 0X0000FF00) << 8) |
        ((x & 0X00FF0000) >> 8) |
        ((x & 0XFF000000) >> 24)
    );
}

static int get_exe_specifications(const void* data, uint64_t* meta_data_size, uint64_t* entry_point, uint32_t* flags){

    const char* _data = data;

    uint32_t magic_number = is_little_endian()? swap32(*(uint32_t*)_data) : *(uint32_t*)_data;
    _data += 4;

    if(magic_number != 0x56505500)
        return 1;
    
    *flags = *(uint32_t*)_data;
    _data += 4;

    *meta_data_size = *(uint64_t*)_data;
    _data += 8 + *meta_data_size;

    *entry_point = *(uint64_t*)_data;

    return 0;
}



#endif // =====================  END OF FILE CORE_HEADER ===========================


#if 0
/*      keeping this here just in case
OpProfile get_inst_op_profile(InstId inst){
    switch (inst)
    {
    case INST_NOP:    return ;
    case INST_HALT:   return ;
    case INST_MOV:    return ;
    case INST_MOVV:   return ;
    case INST_PUSH:   return ;
    case INST_PUSHV:  return ;
    case INST_POP:    return ;
    case INST_READ:   return ;
    case INST_WRITE:  return ;
    case INST_NOT:    return ;
    case INST_AND:    return ;
    case INST_NAND:   return ;
    case INST_OR:     return ;
    case INST_XOR:    return ;
    case INST_BSHIFT: return ;
    case INST_JMP:    return ;
    case INST_JMPIF:  return ;
    case INST_JMPIFN: return ;
    case INST_ADDI:   return ;
    case INST_ADDU:   return ;
    case INST_ADDF:   return ;
    case INST_SUBI:   return ;
    case INST_SUBU:   return ;
    case INST_SUBF:   return ;
    case INST_MULI:   return ;
    case INST_MULU:   return ;
    case INST_MULF:   return ;
    case INST_DIVI:   return ;
    case INST_DIVU:   return ;
    case INST_DIVF:   return ;
    case INST_EQI:    return ;
    case INST_EQU:    return ;
    case INST_EQF:    return ;
    case INST_BIGI:   return ;
    case INST_BIGU:   return ;
    case INST_BIGF:   return ;
    case INST_SMLI:   return ;
    case INST_SMLU:   return ;
    case INST_SMLF:   return ;
    case INST_CASTIU: return ;
    case INST_CASTIF: return ;
    case INST_CASTUI: return ;
    case INST_CASTUF: return ;
    case INST_CASTFI: return ;
    case INST_CASTFU: return ;
    
    default:
        break;
    }
    return -1;
}
*/
#endif