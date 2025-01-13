#include "core.h"
#include "lexer.h"
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

static VPU vpu;

#define as_uint8(ptr)   *(uint8_t*) (ptr)
#define as_uint16(ptr)  *(uint16_t*)(ptr)
#define as_uint32(ptr)  *(uint32_t*)(ptr)
#define as_uint64(ptr)  *(uint64_t*)(ptr)
#define as_int8(ptr)    *(int8_t*)  (ptr)
#define as_int16(ptr)   *(int16_t*) (ptr)
#define as_int32(ptr)   *(int32_t*) (ptr)
#define as_int64(ptr)   *(int64_t*) (ptr)
#define as_float32(ptr) *(float*)   (ptr)
#define as_float64(ptr) *(double*)  (ptr)

// returns the number of instruction to sum to RIP
static inline int64_t perform_inst(Inst inst){

    // the first register operand
    #define R1 (*(Register*)(vpu.register_space + (uint8_t) ((inst & 0XFF00)      >> 8)))
    // the second register operand
    #define R2 (*(Register*)(vpu.register_space +(uint8_t) ((inst & 0XFF0000)    >> 16)))
    // the third register operand
    #define R3 (*(Register*)(vpu.register_space +(uint8_t) ((inst & 0XFF000000)  >> 24)))
    // the first literal operand
    #define L1 (uint16_t) ((inst & 0X00FFFF00) >> 8)
    // the second literal operand
    #define L2 (uint16_t) ((inst & 0XFFFF0000) >> 16)
    // the current stack position
    #define SP (*(Register*)(vpu.register_space + RSP)).as_uint64
    // the instruction position
    #define IP (*(Register*)(vpu.register_space + RIP)).as_uint64

    #define OPERATION(OP, TYPE) R1.as_##TYPE = R2.as_##TYPE OP R3.as_##TYPE
    #define COMPARE(OP, TYPE)   R1.as_uint8 = R2.as_##TYPE OP R3.as_##TYPE
    

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
        vpu.return_status = (GET_OP_HINT(inst) == HINT_REG)? (int) R1.as_int8 : (int) L1;
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
    case INST_MOVC:
        if(R1.as_uint64) R2 = R3;
        return 1;
    case INST_MOVV:
        R1.as_uint64 = L2;
        return 1;
    case INST_MOVN:
        R1.as_uint64 = ~(uint64_t)L2;
        return 1;
    case INST_MOVV16:
        R1.as_uint16 = L2;
        return 1;
    case INST_PUSH:
        vpu.stack[SP++] = (GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1;
        return 1;
    case INST_POP:
        R1.as_uint64 = vpu.stack[--SP];
        return 1;
    case INST_GET:
        R1.as_uint64 = vpu.stack[SP - L2];
        return 1;
    case INST_WRITE:
        vpu.stack[SP - L2] = R1.as_uint64;
        return 1;
    case INST_GSP:
        R1.as_ptr = (uint8_t*)vpu.stack + R2.as_uint64;
        return 1;
    case INST_STATIC:
        vpu.stack[SP++] = (uint64_t)(uintptr_t)(vpu.static_memory + ((GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1));
        return 1;
    case INST_READ8:
        R1.as_uint8 = *(uint8_t*)(R2.as_ptr + R3.as_int64);
        return 1;
    case INST_READ16:
        R1.as_uint16 = *(uint16_t*)(R2.as_ptr + R3.as_int64);
        return 1;
    case INST_READ32:
        R1.as_uint32 = *(uint32_t*)(R2.as_ptr + R3.as_int64);
        return 1;
    case INST_READ:
        R1.as_uint64 = *(uint64_t*)(R2.as_ptr + R3.as_int64);
        return 1;
    case INST_SET8:
        *(uint8_t*)(R1.as_ptr + R3.as_uint64) = R2.as_int8;
        return 1;
    case INST_SET16:
        *(uint16_t*)(R1.as_ptr + R3.as_uint64) = R2.as_int16;
        return 1;
    case INST_SET32:
        *(uint32_t*)(R1.as_ptr + R3.as_uint64) = R2.as_int32;
        return 1;
    case INST_SET:
        *(uint64_t*)(R1.as_ptr + R3.as_uint64) = R2.as_int64;
        return 1;
    case INST_NOT:
        R1.as_uint64 = !R2.as_uint64;
        return 1;
    case INST_NEG:
        R1.as_uint64 = ~R2.as_uint64 | R3.as_uint64;
        return 1;
    case INST_AND:
        R1.as_uint64 = R2.as_uint64 & R3.as_uint64;
        return 1;
    case INST_NAND:
        R1.as_uint64 = ~(R2.as_uint64 & R3.as_uint64);
        return 1;
    case INST_OR:
        R1.as_uint64 = R2.as_uint64 | R3.as_uint64;
        return 1;
    case INST_XOR:
        R1.as_uint64 = R2.as_uint64 ^ R3.as_uint64;
        return 1;
    case INST_BSHIFT:
        R1.as_uint64 = R3.as_int8 < 0? R2.as_uint64 >> -(R3.as_int8) : R2.as_uint64 << R3.as_uint8;
        return 1;
    case INST_JMP:
        return (GET_OP_HINT(inst) == HINT_REG)? R1.as_int64 : (int64_t) L1;
    case INST_JMPF:
        return (R1.as_uint8)? (int16_t) L2 : 1;
    case INST_JMPFN:
        return (!(R1.as_uint8))? (int16_t) L2 : 1;
    case INST_CALL:
        vpu.stack[SP++] = IP + 1;
        return (GET_OP_HINT(inst) == HINT_REG)? R1.as_int64 : (int16_t) L1;
    case INST_RET:
        IP = vpu.stack[--SP];
        return 0;

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

    case INST_NEQ:
        COMPARE(!=, uint64);
        return 1;
    case INST_EQ:
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
        R1.as_int64 = (int64_t)R2.as_uint64;
        return 1;
    case INST_CASTIF:
        R1.as_int64 = (int64_t)R2.as_float64;
        return 1;
    case INST_CASTUI:
        R1.as_uint64 = (uint64_t)R2.as_int64;;
        return 1;
    case INST_CASTUF:
        R1.as_uint64 = (uint64_t)R2.as_float64;
        return 1;
    case INST_CASTFI:
        R1.as_float64 = (double)R2.as_int64;
        return 1;
    case INST_CASTFU:
        R1.as_float64 = (double)R2.as_uint64;
        return 1;
    case INST_CF3264:
        R1.as_float32 = (float)R2.as_float64;
        return 1;
    case INST_CF6432:
        R1.as_float64 = (double)R2.as_float32;
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
        R1.as_uint8 = (uint8_t)memcmp(R1.as_ptr, R2.as_ptr, R3.as_uint64);
        return 1;
    case INST_MALLOC:
        R1.as_ptr = malloc(R2.as_uint64 + R3.as_uint8);
	    R1.as_uint64 += (R1.as_uint64 % (R3.as_uint8 + 1)) % (R3.as_uint8 + 1);
        return 1;
    case INST_FREE:
        free(R1.as_ptr + R2.as_int64 - ((((R1.as_uint64 + R2.as_uint64)) % (R3.as_uint8 + 1)) % (R3.as_uint8 + 1)));
        return 1;
    

    case INST_FOPEN:
        switch (R3.as_uint8)
        {
        default:
        case 0: R1.as_ptr = (uint8_t*)fopen((char*)R2.as_ptr, "r"); break;
        case 1: R1.as_ptr = (uint8_t*)fopen((char*)R2.as_ptr, "w"); break;
        case 2: R1.as_ptr = (uint8_t*)fopen((char*)R2.as_ptr, "rb"); break;
        case 3: R1.as_ptr = (uint8_t*)fopen((char*)R2.as_ptr, "wb"); break;
        }
        return 1;
    case INST_FCLOSE:
        R1.as_uint64 = fclose((FILE*)(R2.as_ptr + R3.as_int64));
        return 1;
    case INST_PUTC:
        R1.as_int32 = fputc(R1.as_int32, (FILE*)(R2.as_ptr + R3.as_int64));
        return 1;
    case INST_GETC:
        R1.as_int32 = fgetc((FILE*)(R2.as_ptr + R3.as_int64));
        return 1;

    case INST_ABS:{
        const int64_t v = R2.as_uint64 - R3.as_uint64;
        R1.as_uint64 = (v < 0)? -v : v;
    }   return 1;
    case INST_ABSF:{
        const double v = R2.as_float64 - R3.as_float64;
        R1.as_float64 = (v < 0)? -v : v;
    }   return 1;
    case INST_INC:
        R1.as_uint64 += L2;
        return 1;
    case INST_DEC:
        R1.as_int64 -= L1;
        return 1;
    case INST_INCF:
        R1.as_float64 += (double)L2;
        return 1;
    case INST_DECF:
        R1.as_float64 -= (double)L2;
        return 1;
    case INST_FLOAT:
        R1.as_float64 = (double) R2.as_int64 / (double) R3.as_uint64;
        return 1;

    case INST_LOAD1:
        R1.as_uint64 = ((vpu.program[IP + 1] & 0XFFFFFF00) << 8) | ((inst & 0XFFFF0000) >> 16);
        return 2;
    case INST_LOAD2:
        R1.as_uint64 = 
            (uint64_t) ((uint64_t) (vpu.program[IP + 2] & 0XFFFFFF00) << 32) |
            ((uint64_t) (vpu.program[IP + 1] & 0XFFFFFF00) << 8) |
            ((uint64_t) (inst & 0XFFFF0000) >> 16);
        return 3;
    
    case INST_IOE:
        R1.as_ptr = (uint8_t*) stdin;
        R2.as_ptr = (uint8_t*) stdout;
        R3.as_ptr = (uint8_t*) stderr;
        return 1;
    

    case INST_SYS:
        if(sys_call(&vpu, (GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1)){
	    fprintf(stderr, "Syscall Failed At IP %"PRIu64"\n", IP);
            vpu.return_status = 1;
	    return 0xFFFFFFFFFFFFFFFF - IP;
	}
        return 1;
    case INST_DISREG:
        printf("(%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", R1.as_uint64, R1.as_uint64, R1.as_int64, R1.as_float64);
        return 1;
    
    
    default:
        fprintf(stderr, "[ERROR] Unknwon Instruction '%u' At Instuction Position %"PRIu64"\n", (unsigned int)inst, IP);
	vpu.return_status = 1;
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

    const uint64_t program_size = (stream.size - meta_data_size - skip - padding) / 4;

    vpu.program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    vpu.register_space = (uint8_t*)vpu.registers;

    vpu.return_status = 0;

    for(
        vpu.registers[RIP / 8].as_uint64 = entry_point;
        vpu.registers[RIP / 8].as_uint64 < program_size;
        vpu.registers[RIP / 8].as_int64 += perform_inst(vpu.program[vpu.registers[RIP / 8].as_uint64])
    ) {
	    vpu.registers[R0].as_uint64 = 0;
    }

    mc_destroy_stream(stream);

    return vpu.return_status;
}

