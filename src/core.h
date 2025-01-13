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
    // R1.8 = *(uint8_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ8,
    // R1.16 = *(uint16_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ16,
    // R1.32 = *(uint32_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ32,
    // R1 = *(uint64_t*)(R2.as_ptr + R3.as_uint64)
    INST_READ,
    // *(uint8_t*)(R1.as_ptr + R3.as_int64) = R2.8
    INST_SET8,
    // *(uint16_t*)(R1.as_ptr + R3.as_int64) = R2.16
    INST_SET16,
    // *(uint32_t*)(R1.as_ptr + R3.as_int64) = R2.32
    INST_SET32,
    // *(uint64_t*)(R1.as_ptr + R3.as_int64) = R2
    INST_SET,
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
    // RIP += L2
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
    // sets R3.as_uint64 bytes to R2.8 starting from R1.as_ptr, sets R1 to NULL on failure
    INST_MEMSET,
    // copy R3.as_uint64 bytes from R1.as_ptr to R2.as_ptr, sets R1 to NULL on failure
    INST_MEMCPY,
    // copy R3.as_uint64 bytes from R1.as_ptr to R2.as_ptr taking into account overlapping strings, sets R1 to NULL on failure
    INST_MEMMOV,
    // compares R3.as_uint64 bytes from R1.as_ptr to R2.as_ptr, sets R1.8 to 1 if all the bytes are equal or 0 otherwise
    INST_MEMCMP,
    // allocates R2.as_uint64 bytes aligned to R3.as_uint8 + 1 and stores it to R1.as_ptr
    INST_MALLOC,
    // frees a block of memory allocated with MALLOC in R1.as_ptr + R2.as_int64 aligned to R3.as_uint8 + 1
    INST_FREE,
    // opens a file given by the string in R2.as_ptr in mode R3.as_uint8 and stores it to R1.as_ptr
    INST_FOPEN,
    // closes a file in R2.as_ptr + R3.as_int64, sets R1.as_int64 to an error value on failure
    INST_FCLOSE,
    // puts ((char)R1.32) to the file at R2.as_ptr + R3.as_int64, sets R1.as_int32 to an error value on failure
    INST_PUTC,
    // gets a byte from the file at R2.as_ptr + R3.as_int64, sets R1.as_int32 to an error value on failure
    INST_GETC,
    // R1.as_uint64 = abs(R2.as_uint64 - R3.as_uint64)
    INST_ABS,
    // R1.as_float64 = abs(R2.as_float64 - R3.as_float64)
    INST_ABSF,
    // R1.as_uint64 += L2.as_uint16
    INST_INC,
    // R1.as_uint64 -= L2.as_uint16
    INST_DEC,
    // R1.as_float64 += (double) L2.as_uint16
    INST_INCF,
    // R1.as_float64 -= (double) L2.as_uint16
    INST_DECF,
    // R1.as_float64 = (double)(R2.as_int64) / (double)(R3.as_uint64)
    INST_FLOAT,
    // loads up to a 32 bit long value to a register
    // R1 = L2.32
    INST_LOAD1,
    // loads up to a 64 bit long value to a register
    // R1 = L2.64
    INST_LOAD2,
    // sets R1.as_ptr, R2.as_ptr and R3.as_ptr to the standard input, output and error, respectively
    INST_IOE,
// -------------------------------------------------------------------------------------------------
    // a dummy instruction that serves to hold immediate values for the LOAD1 and LOAD2 instructions
    INST_CONTAINER = 252,
    // perfomrs a syscall identifies by the value in E
    INST_SYS = 253,
    // displays a register's value, for debugging purposes
    INST_DISREG = 254,
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
