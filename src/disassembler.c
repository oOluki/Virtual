#include "core.h"
#include "lexer.h"
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>

static char buff1[10];
static char buff2[10];
static char buff3[10];
static Inst* program = NULL;

typedef struct Exe
{
    Mc_stream_t data;
    uint64_t    entry_point;
    uint64_t    meta_data_size;
    void*       meta_data;
    uint32_t    flags;
} Exe;


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

    switch (8 * (int)(reg / 8))
    {
    case R0:
	output[0] = 'R';
	output[1] = '0';
	output[2] = get_digit_char(reg - R0);
	output[3] = '\0';
	return output;
    case RA:
        output[0] = 'R';
        output[1] = 'A';
        output[2] = get_digit_char(reg - RA);
        output[3] = '\0';
        return output;
    case RB:
        output[0] = 'R';
        output[1] = 'B';
        output[2] = get_digit_char(reg - RB);
        output[3] = '\0';
        return output;
    case RC:
        output[0] = 'R';
        output[1] = 'C';
        output[2] = get_digit_char(reg - RC);
        output[3] = '\0';
        return output;
    case RD:
        output[0] = 'R';
        output[1] = 'D';
        output[2] = get_digit_char(reg - RD);
        output[3] = '\0';
        return output;
    case RE:
        output[0] = 'R';
        output[1] = 'E';
        output[2] = get_digit_char(reg - RE);
        output[3] = '\0';
        return output;
    case RF:
        output[0] = 'R';
        output[1] = 'F';
        output[2] = get_digit_char(reg - RF);
        output[3] = '\0';
        return output;
    case RSP:
        output[0] = 'R';
        output[1] = 'S';
        output[2] = 'P';
        output[3] = get_digit_char(reg - RSP);
        output[4] = '\0';
        return output;
    case RIP:
        output[0] = 'R';
        output[1] = 'I';
        output[2] = 'P';
        output[3] = get_digit_char(reg - RIP);
        output[4] = '\0';
        return output;
    
    default: return NULL;
    }
}

// \returns 0 on success or 1 otherwise
int print_inst(FILE* output, Inst inst, const uint8_t* static_memory, uint64_t ip){
    #define R1 (uint8_t) ((inst & 0XFF00) >> 8)
    #define R2 (uint8_t) ((inst & 0XFF0000) >> 16)
    #define R3 (uint8_t) (inst >> 24)
    #define L2 (uint16_t) (inst >> 16)
    #define L1 (uint16_t) ((inst & 0X00FFFF00) >> 8)
    switch (inst & 0XFF)
    {
    case INST_NOP:
        fprintf(output, "NOP\n");
        return 0;
    case INST_HALT:
        fprintf(output, "HALT ");
        if((inst >> 24) == HINT_REG){
            fprintf(output, "%s\n", get_reg_str(L1, buff1));
        }
        else fprintf(output, "0x%"PRIx16"; (u: %"PRIi16")\n", L1, L1);
        return 0;
    case INST_MOV8:
        fprintf(output, "MOV8 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV16:
        fprintf(output, "MOV16 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV32:
        fprintf(output, "MOV32 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV:
        fprintf(output, "MOV %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOVC:
        fprintf(output, "MOVC %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MOVV:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "MOVV %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVN:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "MOVN %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVV16:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "MOVV16 %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_PUSH:
        fprintf(output, "PUSH ");
	if(GET_OP_HINT(inst) == HINT_REG)
		fprintf(output, "%s\n", get_reg_str(R1, buff1));
	else {
		const Register op = {.as_uint64 = (uint64_t) L1};
		fprintf(output, "0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
	}
        return 0;
    case INST_POP:
        fprintf(output, "POP %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_GET:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "GET %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_WRITE:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "WRITE %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_GSP:
        fprintf(output, "GSP %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_STATIC:
        if(GET_OP_HINT(inst) == HINT_REG){
	        fprintf(output, "STATIC %s\n", get_reg_str(R1, buff1));
        }
        else {
                const char* string = (char*)(static_memory + L1);
                const uint64_t max_size = *(uint64_t*)(static_memory) - L1;
                fprintf(output, "STATIC 0x%"PRIx16" ;; \"", L1);
                for(int i = 0; i < ((15 < max_size)? (int)15 : (int)max_size); i+=1){
                    if(isprint(string[i])) fputc(string[i], output);
                    else                   fputc('.', output);
                }
                fprintf(output, "\"...\n");
        }   return 0;
    case INST_READ8:
        fprintf(output, "READ8 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_READ16:
        fprintf(output, "READ16 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_READ32:
        fprintf(output, "READ32 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_READ:
        fprintf(output, "READ %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SET8:
        fprintf(output, "SET8 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SET16:
        fprintf(output, "SET16 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SET32:
        fprintf(output, "SET32 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SET:
        fprintf(output, "SET %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_NOT:
        fprintf(output, "NOT %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_NEG:
        fprintf(output, "NEG %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_AND:
        fprintf(output, "AND %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_NAND:
        fprintf(output, "NAND %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_OR:
        fprintf(output, "OR %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_XOR:
        fprintf(output, "XOR %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_BSHIFT:
        fprintf(output, "BSHIFT %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_JMP:
        fprintf(output, "JMP ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff1));
        else
            fprintf(output, "0x%"PRIx16"; %"PRIi16"\n", L1, (int16_t) L1);
        return 0;
    case INST_JMPF:
        fprintf(output, "JMPF %s 0x%"PRIx16"; i: %"PRIi16"\n", get_reg_str(R1, buff1), L2, (int16_t) L2);
        return 0;
    case INST_JMPFN:
        fprintf(output, "JMPFN %s 0x%"PRIx16"; i: %"PRIi16"\n", get_reg_str(R1, buff1), L2, (int16_t) L2);
        return 0;
    case INST_CALL:
        fprintf(output, "CALL ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff1));
        else
            fprintf(output, "0x%"PRIx16"; i: %"PRIi16"\n", L1, (int16_t) L1);
        return 0;
    case INST_RET:
        fprintf(output, "RET\n");
        return 0;
    case INST_ADD8:
        fprintf(output, "ADD8 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SUB8:
        fprintf(output, "SUB8 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MUL8:
        fprintf(output, "MUL8 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ADD16:
        fprintf(output, "ADD16 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SUB16:
        fprintf(output, "SUB16 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MUL16:
        fprintf(output, "MUL16 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ADD32:
        fprintf(output, "ADD32 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SUB32:
        fprintf(output, "SUB32 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MUL32:
        fprintf(output, "MUL32 %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ADD:
        fprintf(output, "ADD %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SUB:
        fprintf(output, "SUB %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MUL:
        fprintf(output, "MUL %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_DIVI:
        fprintf(output, "DIVI %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_DIVU:
        fprintf(output, "DIVU %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ADDF:
        fprintf(output, "ADDF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SUBF:
        fprintf(output, "SUBF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MULF:
        fprintf(output, "MULF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_DIVF:
        fprintf(output, "DIVF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_NEQ:
        fprintf(output, "NEQ %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_EQ:
        fprintf(output, "EQ %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_EQF:
        fprintf(output, "EQF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_BIGI:
        fprintf(output, "BIGI %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_BIGU:
        fprintf(output, "BIGU %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_BIGF:
        fprintf(output, "BIGF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SMLI:
        fprintf(output, "SMLI %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SMLU:
        fprintf(output, "SMLU %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_SMLF:
        fprintf(output, "SMLF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_CASTIU:
        fprintf(output, "CASTIU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTIF:
        fprintf(output, "CASTIF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTUI:
        fprintf(output, "CASTUI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTUF:
        fprintf(output, "CASTUF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTFI:
        fprintf(output, "CASTFI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTFU:
        fprintf(output, "CASTFU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CF3264:
        fprintf(output, "CF3264 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CF6432:
        fprintf(output, "CF6432 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;

    case INST_MEMSET:
        fprintf(output, "MEMSET %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MEMCPY:
        fprintf(output, "MEMCPY %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MEMMOV:
        fprintf(output, "MEMMOV %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MEMCMP:
        fprintf(output, "MEMCMP %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MALLOC:
        fprintf(output, "MALLOC %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_FREE:
        fprintf(output, "FREE %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    
    case INST_FOPEN:
        fprintf(output, "FOPEN %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_FCLOSE:
        fprintf(output, "FCLOSE %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_PUTC:
        fprintf(output, "PUTC %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_GETC:
        fprintf(output, "GETC %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ABS:
        fprintf(output, "ABS %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_ABSF:
        fprintf(output, "ABSF %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_INC:
        fprintf(output, "INC %s 0x%"PRIx16"; u: %"PRIu16"\n", get_reg_str(R1, buff1), L2, L2);
        return 0;
    case INST_DEC:
        fprintf(output, "DEC %s 0x%"PRIx16"; u: %"PRIu16"\n", get_reg_str(R1, buff1), L2, L2);
        return 0;
    case INST_INCF:
        fprintf(output, "INCF %s 0x%"PRIx16"; f: %f\n", get_reg_str(R1, buff1), L2, (float) L2);
        return 0;
    case INST_DECF:
        fprintf(output, "DECF %s 0x%"PRIx16"; f: %f\n", get_reg_str(R1, buff1), L2, (float) L2);
        return 0;
    case INST_FLOAT:
        fprintf(output, "FLOAT %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    
    case INST_LOAD1:{
	const Register v = (Register){
            .as_uint32 = ((uint64_t) (program[ip + 1] & 0XFFFFFF00) << 8) | ((uint64_t) (inst & 0XFFFF0000) >> 16)
	};
        fprintf(output, "LOAD2 %s 0x%"PRIx32"; (u: %"PRIu32"; %"PRIi32"; f: %f)\n", get_reg_str(R1, buff1), v.as_uint32, v.as_uint32, v.as_int32, v.as_float32);
    }   return 0;
    case INST_LOAD2:{
	const Register v = (Register){
	    .as_uint64 = (uint64_t) ((uint64_t) (program[ip + 2] & 0XFFFFFF00) << 32) |
        ((uint64_t) (program[ip + 1] & 0XFFFFFF00) << 8) | ((uint64_t) (inst & 0XFFFF0000) >> 16)
	};
        fprintf(output, "LOAD2 %s 0x%"PRIx64"; (u: %"PRIu64"; %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), v.as_uint64, v.as_uint64, v.as_int64, v.as_float64);
    }   return 0;

    case INST_IOE:
        fprintf(output, "IOE %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    
    case INST_CONTAINER:
        fprintf(output, ";;CONTAINER 0x%"PRIx32"\n", (inst & 0xFFFFFF00) >> 8);
        return 0;
    case INST_DISREG:
        fprintf(output, "DISREG %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_SYS:
        fprintf(output, "SYS ");
	if(GET_OP_HINT(inst) == HINT_REG)
	    fprintf(output, "%s\n", get_reg_str(R1, buff1));
	else
	    fprintf(output, "%"PRIx16"\n", L1);

        return 0;
    
    
    default:
        fprintf(stderr, "[ERROR] Unkonwn Instruction OpCode %u\n", inst & 0xFF);
        return 1;
    }
}


int main(int argc, char** argv){

    #if 0 // X==X (DEBUG) X==X
    argc = 2;
    argv = malloc(argc * sizeof(char*));
    argv[1] = "../output.bin";
    #endif

    if(argc < 2){
        fprintf(stderr, "[ERROR] Expected At Least 1 Argument, Got %i Instead\n", argc - 1);
        return 1;
    }

    Mc_stream_t stream = mc_create_stream(1024);

    if(!read_file(&stream, argv[1], 1, 0)){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", argv[1]);
        mc_destroy_stream(stream);
        return 2;
    }

    uint32_t padding;
    uint64_t flags;
    uint64_t entry_point;
    uint64_t meta_data_size;

    const uint64_t skip = sizeof(uint32_t) + sizeof(padding) + sizeof(flags) + sizeof(entry_point) + sizeof(meta_data_size);

    uint8_t* meta_data = get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags, &padding);

    if(meta_data == NULL){
        fprintf(stderr, "[ERROR] No Meta Data Found\n");
        mc_destroy_stream(stream);
        return 1;
    }

    uint8_t* static_memory = NULL;

    for(size_t i = 0; i + 8 < meta_data_size; ){
        const uint64_t size = *(uint64_t*)(meta_data + i);
        const uint64_t id   = *(uint64_t*)(meta_data + i + sizeof(uint64_t));
        if(id == is_little_endian()? mc_swap64(0x5354415449433a) : 0x5354415449433a){
            static_memory = meta_data + i;
            break;
        }
        i+=size;
    }
    const uint64_t static_memory_size = *(uint64_t*)(static_memory);

    const uint64_t inst_count = (stream.size - skip - meta_data_size - padding) / sizeof(Inst);

    FILE* output = stdout;

    for(int i = 2; i < argc; i+=1){
        if(mc_compare_str(argv[i], "-o", 0)){
            if(argc <= i + 1){
                fprintf(stderr, "[ERROR] Missing Output File After Flag \"-o\"\n");
                mc_destroy_stream(stream);
                return 1;
            }
            output = fopen(argv[i + 1], "w");
            if(!output){
                fprintf(stderr, "[ERROR] Could Not Open Output File '%s'\n", argv[i + 1]);
                mc_destroy_stream(stream);
                return 1;
            }
            break;
        }
    }

    fprintf( output,
        "\n;; X====X (SPECIFICATIONS) X====X\n"
        "\t;; name = %s\n"
        "\t;; executable size = %"PRIu64"\n"
        "\t;; inst count      = %"PRIu64"\n"
        "\t;; inst size       = %"PRIu32"\n"
        "\t;; flags           = %02"PRIx64"\n"
        "\t;; meta_data_size  = %"PRIu64"\n"
        "\t;; static_memory   = { position = %"PRIu64", size = %"PRIu64" }\n"
        "\t;; entry point     = %"PRIu64"\n"
        ";; X====X (SPECIFICATIONS) X====X\n\n\n",
        argv[1],
        stream.size,
        inst_count,
        (uint32_t) sizeof(Inst),
        flags,
        meta_data_size,
        (uint64_t)(size_t)(static_memory - meta_data), static_memory_size,
        entry_point
    );
    
    if(static_memory_size > 16){
    	fprintf(output, "%cstatic 0x", '%');
        for(uint64_t i = 16; i < static_memory_size; i+=1){
            fprintf(output, "%02"PRIx8"", static_memory[i]);
        }
        fprintf(output, "\n");
    }


    program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    int status = 0;
    uint64_t i = 0;
    for( ; (i < entry_point) && !status; i += 1)
        status = print_inst(output, program[i], static_memory, i);
    if(!status) fprintf(output, "%s\n", "%start");
    for( ; (i < inst_count) && !status; i += 1)
        status = print_inst(output, program[i], static_memory, i);
    
    if(argc == 3) fclose(output);

    if(status) fprintf(stderr, "[ERROR] At Instruction Position %" PRIu64 " ^^^\n", i);

    return status;
}




