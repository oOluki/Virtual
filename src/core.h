#ifndef CORE_HEADER
#define CORE_HEADER

/*
 * QUICK MANUAL:
 * Rn stands for the nth argument which is a register in this case
 * Ln stands for the nth argument which is a literal in this case
 * E stands for the only argument which can be either a literal or a register
 * the .as_<type> suffix indicates that the value is to be read as the <type>, which also means that if the argument is a register it only needs sizeof(<type>) bytes.
 * the .<size> suffix indicates that only the first <size> less significant bytes will be taken into account
 * if no suffix is provided the default is .64 for registers and .16 for literals (as literals have to be up to 16 bit, saved for the LOAD1 and LOAD2 arguments)
 * STACK refers to the stack (64 bit)
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifndef VPU_MEMALIGN_TO
    #define VPU_MEMALIGN_TO 8
#endif

typedef enum ExeFlags{

    EXE_DEFAULT = 0,

} ExeFlags;


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
    // R1 = STACK[RSP - L2]
    INST_GET,
    // STACK[RSP - L2] = R1
    INST_WRITE,
    // R1 = (uint8_t*)(STACK_POINTER) + R2.as_uint64
    INST_GSP,
    // STACK[RSP++] = STATIC_POINTER + E.as_uint64
    INST_STATIC,
    // R1.8 = *((uint8_t*)(R2.as_ptr) + R3.as_uint64);
    INST_READ8,
    // R1.16 = *((uint16_t*)(R2.as_ptr) + R3.as_uint64);
    INST_READ16,
    // R1.32 = *((uint32_t*)(R2.as_ptr) + R3.as_uint64);
    INST_READ32,
    // R1 = *((uint64_t*)(R2.as_ptr) + R3.as_uint64);
    INST_READ,
    // R1.8 = *((uint8_t*)(R2.as_ptr) + R3.as_uint64);
    INST_SET8,
    INST_SET16,
    INST_SET32,
    INST_SET,

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

    INST_NEQ,
    INST_EQ,
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

    INST_ABS,
    INST_ABSF,
    INST_INC,
    INST_DEC,
    INST_INCF,
    INST_DECF,
    
    INST_FLOAT,

    INST_LOAD1,
    INST_LOAD2,

    INST_IOE,

    INST_CONTAINER = 252,

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
    // LITERAL OR REGISTER
    EXPECT_OP_EITHER,
    EXPECT_OPTIONAL,
};

// R: REGISTER, L: LITERAL, E: EITHER LITERAL OR REGISTER ID
typedef enum OpProfile{

    OP_PROFILE_NONE = EXPECT_ANY,
    // instruction takes one register
    OP_PROFILE_R = EXPECT_OP_REG,
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


enum RegisterId{
    R0 = 0 ,
    RA = 8 , RA1, RA2, RA3, RA4,
    RB = 16, RB1, RB2, RB3, RB4,
    RC = 24, RC1, RC2, RC3, RC4,
    RD = 32, RD1, RD2, RD3, RD4,
    RE = 40, RE1, RE2, RE3, RE4,
    RF = 48,

    RSP = 56,
    RIP = 64,
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
    uint8_t*    as_ptr;
} Register;

typedef struct VPU
{

    int      return_status;

    uint8_t* static_memory;

    uint8_t* internal_data;

    Register registers[9];

    uint8_t* register_space;

    Inst*    program;

    uint64_t stack[1000];
    
} VPU;

#define GET_OP_HINT(INST) (INST >> 31)

static inline int is_little_endian(){ return (*(unsigned short *)"\x01\x00" == 0x01); }

static inline uint32_t swap32(uint32_t x){
    return (
        ((x & 0X000000FF) << 24) |
        ((x & 0X0000FF00) << 8) |
        ((x & 0X00FF0000) >> 8) |
        ((x & 0XFF000000) >> 24)
    );
}

static inline void* vpu_alloc_aligned(size_t n, size_t alignment){

    // it may be a good idea to only allow alignment to powers of 2
    //if(alignment & (alignment - 1)) return NULL;

    const uintptr_t addr = (uintptr_t)malloc(n + sizeof(void*) + alignment) + sizeof(void*);

    if(addr == sizeof(void*)) return NULL;

    *(void**)(addr - sizeof(void*)) = (void*)(addr - sizeof(void*));
    const uintptr_t offset = (alignment - (addr % alignment)) % alignment;

    return (void *)(addr + offset);
}

static inline void vpu_free_aligned(void* ptr){

    if(ptr) free((void*) ((uintptr_t)(ptr) - sizeof(void*)));

}

static void* get_exe_specifications(const void* data, uint64_t* meta_data_size, uint64_t* entry_point, uint64_t* flags, uint32_t* padding){

    const char* _data = data;

    uint32_t magic_number = is_little_endian()? swap32(*(uint32_t*)_data) : *(uint32_t*)_data;
    _data += 4;

    //                     VPU:
    if(magic_number != 0x5650553a)
        return NULL;

    *padding = *(uint32_t*)(_data);
    _data += 4;
    
    *flags = *(uint64_t*)_data;
    _data += 8;

    *entry_point = *(uint64_t*)_data;
    _data += 8;

    *meta_data_size = *(uint64_t*)_data;

    return (void*)(_data + 8);
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
