#ifndef CORE_C
#define CORE_C

#include "core.h"


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













#endif