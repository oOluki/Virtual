#include "core.h"
#include "lexer.h"
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

static VPU vpu;

// returns the number of instruction to sum to RIP
static inline int64_t perform_inst(Inst inst){

    // the first register operand
    #define R1 (*(Register*)(vpu.register_space + (uint8_t) ((inst & 0XFF00)      >> 8)))
    // the second register operand
    #define R2 (*(Register*)(vpu.register_space +(uint8_t) ((inst & 0XFF0000)    >> 16)))
    // the third register operand
    #define R3 (*(Register*)(vpu.register_space +(uint8_t) ((inst & 0XFF000000)  >> 24)))
    // the first literal operand
    #define L1 (uint32_t) ((inst & 0XFFFFFF00) >> 8)
    // the second literal operand
    #define L2 (uint16_t) ((inst & 0XFFFF0000) >> 16)
    // the current stack position
    #define SP (*(Register*)(vpu.register_space + RSP)).as_uint64
    // the instruction position
    #define IP (*(Register*)(vpu.register_space + RIP)).as_uint64

    #define OPERATION(OP, TYPE) R1.as_##TYPE = R1.as_##TYPE OP R2.as_##TYPE
    #define COMPARE(OP, TYPE)   R1.as_uint8 = R1.as_##TYPE OP R2.as_##TYPE

    switch (inst & 0XFF)
    {

/*--------------------------------------------------------------------------------------------------------------/
/                                                                                                               /
/               BASIC LOGIC                               BASIC LOGIC                                           /
/                                                                                                               /
/--------------------------------------------------------------------------------------------------------------*/

    case INST_NOP:
        return 1;
    case INST_HALT:
        return 0XFFFFFFFFFFFFFFFF - IP;
    case INST_MOV8:
        R1.as_uint8 = R2.as_uint8;
        return 1;
    case INST_MOV16:
        R1.as_uint16 = R2.as_uint16;
        return 1;
    case INST_MOV32:
        R1.as_uint32 = R2.as_uint32;
        return 1;
    case INST_MOV:
        R1 = R2;
        return 1;
    case INST_MOVV:
        R1.as_uint64 = L2;
        return 1;
    case INST_PUSH:
        vpu.stack[SP++] = R1.as_uint64;
        return 1;
    case INST_PUSHV:
        vpu.stack[SP++] = L1;
       return 1;
    case INST_POP:
        R1.as_uint64 = vpu.stack[--SP];
        return 1;
    case INST_GET:
        R1.as_uint64 = vpu.stack[SP - R2.as_uint64];
        return 1;
    case INST_WRITE:
        vpu.stack[SP - L2] = R1.as_uint64;
        return 1;
    case INST_GSP:
        R1.as_ptr = vpu.stack;
        return 1;
    case INST_STATIC:
        vpu.stack[SP++] = (uint64_t)(uintptr_t)(vpu.static_memory + L1);
        return 1;
    case INST_READ8:
        R1.as_uint8 = *(uint8_t*)(R2.as_ptr);
        return 1;
    case INST_READ16:
        R1.as_uint16 = *(uint16_t*)(R2.as_ptr);
        return 1;
    case INST_READ32:
        R1.as_uint32 = *(uint32_t*)(R2.as_ptr);
        return 1;
    case INST_READ:
        R1.as_uint64 = *(uint64_t*)(R2.as_ptr);
        return 1;
    case INST_SET8:
        *(uint8_t*)(R1.as_ptr) = R2.as_uint8;
        return 1;
    case INST_SET16:
        *(uint16_t*)(R1.as_ptr) = R2.as_uint16;
        return 1;
    case INST_SET32:
        *(uint32_t*)(R1.as_ptr) = R2.as_uint32;
        return 1;
    case INST_SET:
        *(uint64_t*)(R1.as_ptr) = R2.as_uint64;
        return 1;
    case INST_TEST:
        R1.as_uint64 = (R1.as_uint64 != 0);
        return 1;
    case INST_NOT:
        R1.as_uint64 = !R1.as_uint64;
        return 1;
    case INST_NEG:
        R1.as_uint64 = ~R1.as_uint64;
        return 1;
    case INST_AND:
        R1.as_uint64 = R1.as_uint64 & R2.as_uint64;
        return 1;
    case INST_NAND:
        R1.as_uint64 = ~(R1.as_uint64 & R2.as_uint64);
        return 1;
    case INST_OR:
        R1.as_uint64 = R1.as_uint64 | R2.as_uint64;
        return 1;
    case INST_XOR:
        R1.as_uint64 = R1.as_uint64 ^ R2.as_uint64;
        return 1;
    case INST_BSHIFT:
        R1.as_uint64 = R2.as_int64 < 0? R1.as_uint64 >> -(R2.as_int64) : R1.as_uint64 << R2.as_uint64;
        return 1;
    case INST_JMP:
        IP = L1;
        return 1;
    case INST_JMPIF:
        if(R1.as_uint8){
            IP = L2;
            return 0;
        }
        return 1;
    case INST_JMPIFN:
        if(!(R1.as_uint8)){
            IP = L2;
            return 0;
        }
        return 1;
    case INST_CALL:
        vpu.stack[SP++] = IP + 2;
        IP = R1.as_uint64;
        return 1;
    case INST_RET:
        IP = vpu.stack[--SP];
        return 1;

/*--------------------------------------------------------------------------------------------------------------/
/                                                                                                               /
/               ARITHMETIC OPERATIONS                     ARITHMETIC OPERATIONS                                 /
/                                                                                                               /
/--------------------------------------------------------------------------------------------------------------*/


    case INST_ADD8:
        OPERATION(+, uint8);
        return 1;
    case INST_SUB8:
        OPERATION(-, uint8);
        return 1;
    case INST_MUL8:
        OPERATION(*, uint8);
        return 1;
    case INST_ADD16:
        OPERATION(+, uint16);
        return 1;
    case INST_SUB16:
        OPERATION(-, uint16);
        return 1;
    case INST_MUL16:
        OPERATION(*, uint16);
        return 1;
    case INST_ADD32:
        OPERATION(+, uint32);
        return 1;
    case INST_SUB32:
        OPERATION(-, uint32);
        return 1;
    case INST_MUL32:
        OPERATION(*, uint32);
        return 1;
    case INST_ADD:
        OPERATION(+, uint64);
        return 1;
    case INST_SUB:
        OPERATION(-, uint64);
        return 1;
    case INST_MUL:
        OPERATION(*, uint64);
        return 1;
    case INST_DIVI:
        OPERATION(/, int64);
        return 1;
    case INST_DIVU:
        OPERATION(/, uint64);
        return 1;
    case INST_ADDF:
        OPERATION(+, float64);
        return 1;
    case INST_SUBF:
        OPERATION(-, float64);
        return 1;
    case INST_MULF:
        OPERATION(*, float64);
        return 1;
    case INST_DIVF:
        OPERATION(/, float64);
        return 1;

/*--------------------------------------------------------------------------------------------------------------/
/                                                                                                               /
/               ARITHMETIC COMPARISSONS                     ARITHMETIC COMPARISSONS                             /
/                                                                                                               /
/--------------------------------------------------------------------------------------------------------------*/

    case INST_EQI:
        COMPARE(==, int64);
        return 1;
    case INST_EQU:
        COMPARE(==, uint64);
        return 1;
    case INST_EQF:
        COMPARE(==, float64);
        return 1;
    case INST_BIGI:
        COMPARE(>, int64);
        return 1;
    case INST_BIGU:
        COMPARE(>, uint64);
        return 1;
    case INST_BIGF:
        COMPARE(>, float64);
        return 1;
    case INST_SMLI:
        COMPARE(<, int64);
        return 1;
    case INST_SMLU:
        COMPARE(<, uint64);
        return 1;
    case INST_SMLF:
        COMPARE(<, float64);
        return 1;


    case INST_CASTIU:
        R1.as_int64 = (int64_t)R1.as_uint64;
        return 1;
    case INST_CASTIF:
        R1.as_int64 = (int64_t)R1.as_float64;
        return 1;
    case INST_CASTUI:
        R1.as_uint64 = (uint64_t)R1.as_int64;;
        return 1;
    case INST_CASTUF:
        R1.as_uint64 = (uint64_t)R1.as_float64;
        return 1;
    case INST_CASTFI:
        R1.as_float64 = (uint64_t)R1.as_int64;
        return 1;
    case INST_CASTFU:
        R1.as_float64 = (uint64_t)R1.as_uint64;
        return 1;
    case INST_CF3264:
        R1.as_float32 = (float)R1.as_float64;
        return 1;
    case INST_CF6432:
        R1.as_float64 = (double)R1.as_float32;
        return 1;

    
    case INST_MEMSET:
        R1.as_ptr = memset(R1.as_ptr, R2.as_uint8, R3.as_uint64);
        return 1;
    case INST_MEMCPY:
        R1.as_ptr = memcpy(R1.as_ptr, R2.as_ptr, R3.as_uint64);
        return 1;
    case INST_MEMMOV:
        R1.as_ptr = memmove(R1.as_ptr, R2.as_ptr, R3.as_uint64);
        return 1;
    case INST_MEMCMP:
        R1.as_int8 = (uint8_t)memcmp(R1.as_ptr, R2.as_ptr, R3.as_uint64);
        return 1;
    case INST_MALLOC:
        R1.as_ptr = malloc(R1.as_uint64);
        return 1;
    case INST_FREE:
        free(R1.as_ptr);
        return 1;
    

    case INST_FOPEN:
        switch (R2.as_uint8)
        {
        case 0: R1.as_ptr = fopen(R1.as_ptr, "rb"); break;
        case 1: R1.as_ptr = fopen(R1.as_ptr, "wb"); break;
        default:
        case 2: R1.as_ptr = fopen(R1.as_ptr, "r"); break;
        case 3: R1.as_ptr = fopen(R1.as_ptr, "w"); break;
        }
        return 1;
    case INST_FCLOSE:
        R1.as_uint32 = fclose((FILE*)R1.as_ptr);
        return 1;
    case INST_PUTC:
        R1.as_int32 = fputc(R1.as_int32, R2.as_ptr);
        return 1;
    case INST_GETC:
        R1.as_int32 = fgetc(R1.as_ptr);
        return 1;
    

    case INST_SYS:
        if(sys_call(&vpu, L1)){
            exit(1);
        }
        return 1;
    case INST_DISREG:
        printf("(%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", R1.as_uint64, R1.as_uint64, R1.as_int64, R1.as_float64);
        return 1;
    
    
    default:
        fprintf(stderr, "[ERROR] Unknwon Instruction '%u' At Instuction Position %"PRIu64"\n", (unsigned int)inst, IP);
        return 0xFFFFFFFFFFFFFFFF - IP;
    }

}



int main(int argc, char** argv){

    if(argc != 2){
        fprintf(stderr, "[ERROR] Expected 1 Argument, Got %i Instead\n", argc - 1);
        return 1;
    }

    Mc_stream_t stream = (Mc_stream_t){.data = NULL, .size = 0, .capacity = 0};

    if(!read_file(&stream, argv[1], 1, 0)){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", argv[1]);
        return 2;
    }



    uint64_t flags;
    uint64_t meta_data_size;
    uint64_t entry_point;
    uint32_t padding;

    const uint64_t skip = sizeof(uint32_t) + sizeof(padding) + sizeof(flags) + sizeof(entry_point) + sizeof(meta_data_size);

    void* meta_data = get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags, &padding);

    if(!meta_data) return 1;

    for(size_t i = 0; i + 8 < meta_data_size; ){
        const uint64_t block_size = *(uint64_t*)((uint8_t*)(stream.data) + i);
        const uint64_t id = *(uint64_t*)((uint8_t*)(meta_data) + i + sizeof(block_size));
        if(id == is_little_endian()? mc_swap64(0x5354415449433a) : 0x5354415449433a){
            vpu.static_memory = (uint8_t*)(meta_data) + i;
            break;
        }
        i += block_size;
    }

    const uint64_t program_size = stream.size - meta_data_size - skip - padding;

    vpu.program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    vpu.register_space = (uint8_t*)vpu.registers;

    for(
        vpu.registers[RIP / 8].as_uint64 = entry_point;
        vpu.registers[RIP / 8].as_uint64 < program_size;
        vpu.registers[RIP / 8].as_uint64 += perform_inst(vpu.program[vpu.registers[RIP / 8].as_uint64])
    );

    mc_destroy_stream(stream);

    return 0;
}

