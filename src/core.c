#ifndef CORE_C
#define CORE_C

#include "core.h"
#include "virtual_files.h"
#include "system.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>

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



// returns the number of instruction to sum to RIP
int64_t perform_inst(VPU* vpu, Inst inst){

    Register* const registers = (Register*) vpu->register_space;

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
    
    registers[R0 >> 3].as_uint64 = 0;

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
        R1.as_uint8 = *(uint8_t*)((uintptr_t)(R2.as_ptr) + R3.as_int64);
        return 1;
    case INST_READ16:
        R1.as_uint16 = *(uint16_t*)((uintptr_t)(R2.as_ptr) + R3.as_int64);
        return 1;
    case INST_READ32:
        R1.as_uint32 = *(uint32_t*)((uintptr_t)(R2.as_ptr) + R3.as_int64);
        return 1;
    case INST_READ:
        R1.as_uint64 = *(uint64_t*)((uintptr_t)(R2.as_ptr) + R3.as_int64);
        return 1;
    case INST_MREADS:
        R1.as_ptr = memcpy(R1.as_ptr, R2.as_ptr, (size_t) R3.as_uint64);
        return 1;
    case INST_WRITE8:
        *(uint8_t*)((uintptr_t)(R1.as_ptr) + R3.as_int64) = R2.as_uint8;
        return 1;
    case INST_WRITE16:
        *(uint16_t*)((uintptr_t)(R1.as_ptr) + R3.as_int64) = R2.as_uint16;
        return 1;
    case INST_WRITE32:
        *(uint32_t*)((uintptr_t)(R1.as_ptr) + R3.as_int64) = R2.as_uint32;
        return 1;
    case INST_WRITE:
        *(uint64_t*)((uintptr_t)(R1.as_ptr) + R3.as_int64) = R2.as_uint64;
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
        if(virtual_syscall(vpu, (GET_OP_HINT(inst) == HINT_REG)? R1.as_uint64 : L1)){
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
	case INST_GRP:
	    R1.as_ptr = ((uint8_t*) &R2) + R3.as_int64;
        return 1;
    case INST_GIP:
        R1.as_ptr = ((uint8_t*) (vpu->program + R2.as_uint64)) + R3.as_uint64;
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

#endif
