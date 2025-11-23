#ifndef _VPU_EXE
#define _VPU_EXE

#include "core.h"
#include "lexer.h"
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "virtual_files.h"


// returns the number of instruction to sum to RIP
int64_t perform_inst(VPU* vpu, Inst inst){

    // the first register operand
    #define R1 (*(Register*)(vpu->register_space + (uint8_t) ((inst & 0XFF00)      >> 8)))
    // the second register operand
    #define R2 (*(Register*)(vpu->register_space + (uint8_t) ((inst & 0XFF0000)    >> 16)))
    // the third register operand
    #define R3 (*(Register*)(vpu->register_space + (uint8_t) ((inst & 0XFF000000)  >> 24)))
    // the first literal operand
    #define L1 (uint16_t) ((inst & 0X00FFFF00) >> 8)
    // the second literal operand
    #define L2 (uint16_t) ((inst & 0XFFFF0000) >> 16)
    // the current stack position
    #define SP (*(Register*)(vpu->register_space + RSP)).as_uint64
    // the instruction position
    #define IP (*(Register*)(vpu->register_space + RIP)).as_uint64

    #define OPERATION(OP, TYPE) R1.as_##TYPE = R2.as_##TYPE OP R3.as_##TYPE
    #define COMPARE(OP, TYPE)   R1.as_uint8 = R2.as_##TYPE OP R3.as_##TYPE
    
    vpu->registers[R0 >> 3].as_uint64 = 0;

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
        vpu->status = (GET_OP_HINT(inst) == HINT_REG)? (int) R1.as_int8 : (int) L1;
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
        vpu->stack[SP++] = (GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1;
        return 1;
    case INST_POP:
        R1.as_uint64 = vpu->stack[--SP];
        return 1;
    case INST_STACK_GET:
        R1.as_uint64 = vpu->stack[SP - L2];
        return 1;
    case INST_STACK_PUT:
        vpu->stack[SP - L2] = R1.as_uint64;
        return 1;
    case INST_GSP:
        R1.as_ptr = (uint8_t*)((uint64_t*) vpu->stack + R2.as_uint64) + R3.as_uint64;
        return 1;
    case INST_STATIC:
        vpu->stack[SP++] = (uint64_t)(uintptr_t)(vpu->static_memory + ((GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1));
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
    case INST_WRITE8:
        *(uint8_t*)(R1.as_ptr + R3.as_int64) = R2.as_uint8;
        return 1;
    case INST_WRITE16:
        *(uint16_t*)(R1.as_ptr + R3.as_int64) = R2.as_uint16;
        return 1;
    case INST_WRITE32:
        *(uint32_t*)(R1.as_ptr + R3.as_int64) = R2.as_uint32;
        return 1;
    case INST_WRITE:
        *(uint64_t*)(R1.as_ptr + R3.as_int64) = R2.as_uint64;
        return 1;
    case INST_MWRITES:
        R1.as_ptr = memset(R1.as_ptr, (int) R2.as_int8, (size_t) R3.as_uint64);
        return 1;
    case INST_MMOVS:
        R1.as_ptr = memmove(R1.as_ptr,  R2.as_ptr, (size_t) R3.as_uint64);
        return 1;
    case INST_MEMCMP:
        R1.as_uint8 = (uint8_t) memcmp(R1.as_ptr, R2.as_ptr, (size_t) R3.as_uint64);
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
        return (GET_OP_HINT(inst) == HINT_REG)? R1.as_int64 : (int16_t) L1;
    case INST_JMPF:
        return (R1.as_uint8)? (int16_t) L2 : 1;
    case INST_JMPFN:
        return (!(R1.as_uint8))? (int16_t) L2 : 1;
    case INST_CALL:
        vpu->stack[SP++] = IP + 1;
        return (GET_OP_HINT(inst) == HINT_REG)? R1.as_int64 : (int16_t) L1;
    case INST_RET:
        IP = vpu->stack[--SP];
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
    case INST_INC:
        R1.as_uint64 += L2;
        return 1;
    case INST_DEC:
        R1.as_int64 -= L2;
        return 1;
    case INST_INCF:
        R1.as_float64 += (double)L2;
        return 1;
    case INST_DECF:
        R1.as_float64 -= (double)L2;
        return 1;
    case INST_ABS:{
        const int64_t v = R2.as_int64 - R3.as_int64;
        R1.as_uint64 = (v < 0)? -v : v;
    }   return 1;
    case INST_ABSF:{
        const double v = R2.as_float64 - R3.as_float64;
        R1.as_float64 = (v < 0)? -v : v;
    }   return 1;

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
    case INST_FLOAT:
        R1.as_float64 = (double) R2.as_int64 / (double) R3.as_uint64;
        return 1;

    case INST_DUMPCHAR:
        if(R2.as_int8 == 0){
            putchar((int) R1.as_int32);
            if(R3.as_uint8) fflush(stdout);
        }
        else{
            fputc((int) R1.as_int32, stderr);
            if(R3.as_uint8) fflush(stdout);
        }
        return 1;
    case INST_GETCHAR:
        R1.as_int32 = fgetc(stdin);
        if(R2.as_uint8) fclose(stdin);
        return 1;
    case INST_EXEC:
        return perform_inst(vpu, R3.as_uint32);
    case INST_SYS:
        if(sys_call(vpu, (GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1)){
            fprintf(stderr, "Syscall Failed At IP %"PRIu64"\n", IP);
            vpu->status = 1;
            return 0xFFFFFFFFFFFFFFFF - IP;
	    }
        return 1;
    case INST_DISREG:{
            char buff[8];
            if((inst >> 8 ) & 0xFF)
                printf("%s = (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str((inst >> 8 ) & 0xFF, buff), R1.as_uint64, R1.as_uint64, R1.as_int64, R1.as_float64);
            if((inst >> 16) & 0xFF)
                printf("%s = (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str((inst >> 16) & 0xFF, buff), R2.as_uint64, R2.as_uint64, R2.as_int64, R2.as_float64);
            if((inst >> 24) & 0xFF)
                printf("%s = (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str((inst >> 24) & 0xFF, buff), R3.as_uint64, R3.as_uint64, R3.as_int64, R3.as_float64);
        }
        return 1;
    
    
    default:
        fprintf(stderr, "[ERROR] Unknwon Instruction '%u' At Instuction Position %"PRIu64"\n", (unsigned int)inst, IP);
	    vpu->status = 1;
        return 0xFFFFFFFFFFFFFFFF - IP;
    }

    #undef R1
    #undef R2
    #undef R3
    #undef L1
    #undef L2
    #undef SP
    #undef IP
    #undef OPERATION
    #undef COMPARE

}

// executes raw program and passes argc and argv to the executing program
int execute(const char* input_file, int argc, char** argv){

    if(!input_file){
        fprintf(stderr, "[ERROR] Expected Input Program Path\n");
        return 1;
    }

    VirtualFile vfile;
    const char* required_fields[] = {
        VIRTUAL_FILE_PROGRAM_FIELD_NAME,
        NULL
    };
    const char* optional_fields[] = {
        VIRTUAL_FILE_STATIC_FIELD_NAME,
        NULL
    };
    if(vfopen(&vfile, input_file, required_fields, optional_fields)){
        fprintf(stderr, "[ERROR] failed trying to open virtual file '%s'\n", input_file);
        return 1;
    }

    const void* const program_field = get_virtual_file_field(vfile, VIRTUAL_FILE_PROGRAM_FIELD_NAME);
    
    VPU vpu;

    uint64_t entry_point;
    uint64_t program_size;
    vpu.program = get_program_from_vfield(program_field, &program_size, &entry_point);
    if(program_field == NULL){
        fprintf(stderr, "[ERROR] virtual file in '%s' has corrupt program\n", input_file);
        vfclose(vfile);
        return 1;
    }

    static uint64_t stack[1000];
    Register registers[REGISTER_SPACE_SIZE / sizeof(Register)];
    memset(stack, 0, sizeof(stack));
    memset(registers, 0, sizeof(registers));

    vpu.static_memory = (uint8_t*) get_virtual_file_field(vfile, VIRTUAL_FILE_STATIC_FIELD_NAME);
    if(vpu.static_memory){
        vpu.static_memory = (uint8_t*) (((uintptr_t) vpu.static_memory) + sizeof(uint64_t) + sizeof(VIRTUAL_FILE_STATIC_FIELD_NAME));
    }
    
    vpu.stack = &(stack[0]);
    vpu.registers = &(registers[0]);

    // sets argc and argv of the program to RA.as_int64 and RB.as_ptr, respectively
    vpu.registers[RA >> 3].as_int64 = argc;
    vpu.registers[RB >> 3].as_ptr   = (uint8_t*) argv;

    vpu.register_space = (uint8_t*)vpu.registers;

    vpu.status = 0;

    for(
        vpu.registers[RIP >> 3].as_uint64 = entry_point;
        vpu.registers[RIP >> 3].as_uint64 < program_size;
        vpu.registers[RIP >> 3].as_int64 += perform_inst(&vpu, vpu.program[vpu.registers[RIP >> 3].as_uint64])
    ) {
	    //vpu.registers[R0].as_uint64 = 0;
    }

    vfclose(vfile);

    return vpu.status;
}

#endif // END OF FILE