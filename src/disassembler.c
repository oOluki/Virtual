#ifndef _VPU_DISASSEMBLER
#define _VPU_DISASSEMBLER

#include "core.h"
#include "lexer.h"
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include "labels.h"


// \param buff should be an array of 3 buffers of size 8 bytes each
// \returns 0 on success or 1 otherwise
int print_inst(FILE* output, const Inst* program, const uint8_t* static_memory, uint64_t ip, char** buff){
    #define R1 (uint8_t) ((inst & 0XFF00) >> 8)
    #define R2 (uint8_t) ((inst & 0XFF0000) >> 16)
    #define R3 (uint8_t) (inst >> 24)
    #define L2 (uint16_t) (inst >> 16)
    #define L1 (uint16_t) ((inst & 0X00FFFF00) >> 8)

    const Inst inst = program[ip];

    switch (inst & 0XFF)
    {
    case INST_NOP:
        fprintf(output, "\tNOP\n");
        return 0;
    case INST_HALT:
        fprintf(output, "\tHALT ");
        if((inst >> 24) == HINT_REG){
            fprintf(output, "%s\n", get_reg_str(L1, buff[0]));
        }
        else fprintf(output, "\t0x%"PRIx16"; (u: %"PRIi16")\n", L1, L1);
        return 0;
    case INST_MOV8:
        fprintf(output, "\tMOV8 %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_MOV16:
        fprintf(output, "\tMOV16 %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_MOV32:
        fprintf(output, "\tMOV32 %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_MOV:
        fprintf(output, "\tMOV %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_MOVC:
        fprintf(output, "\tMOVC %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MOVV:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tMOVV %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVN:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tMOVN %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVV16:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tMOVV16 %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_PUSH:
        fprintf(output, "\tPUSH ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff[0]));
        else {
            const Register op = {.as_uint64 = (uint64_t) L1};
            fprintf(output, "0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
        }
        return 0;
    case INST_POP:
        fprintf(output, "\tPOP %s\n", get_reg_str(R1, buff[0]));
        return 0;
    case INST_GET:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tGET %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_WRITE:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tWRITE %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_GSP:
        fprintf(output, "\tGSP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_STATIC:
        if(GET_OP_HINT(inst) == HINT_REG){
	        fprintf(output, "\tSTATIC %s\n", get_reg_str(R1, buff[0]));
        }
        else {
                const char* string = (char*)(static_memory + L1);
                const uint64_t max_size = *(uint64_t*)(static_memory) - L1;
                fprintf(output, "\tSTATIC 0x%"PRIx16" ;; \"", L1);
                for(int i = 0; i < ((15 < max_size)? (int)15 : (int)max_size); i+=1){
                    if(isprint(string[i])) fputc(string[i], output);
                    else                   fputc('.', output);
                }
                fprintf(output, "\"...\n");
        }   return 0;
    case INST_READ8:
        fprintf(output, "\tREAD8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_READ16:
        fprintf(output, "\tREAD16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_READ32:
        fprintf(output, "\tREAD32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_READ:
        fprintf(output, "\tREAD %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SET8:
        fprintf(output, "\tSET8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SET16:
        fprintf(output, "\tSET16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SET32:
        fprintf(output, "\tSET32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SET:
        fprintf(output, "\tSET %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_NOT:
        fprintf(output, "\tNOT %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_NEG:
        fprintf(output, "\tNEG %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_AND:
        fprintf(output, "\tAND %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_NAND:
        fprintf(output, "\tNAND %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_OR:
        fprintf(output, "\tOR %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_XOR:
        fprintf(output, "\tXOR %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_BSHIFT:
        fprintf(output, "\tBSHIFT %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_JMP:
        fprintf(output, "\tJMP ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff[0]));
        else
            fprintf(output, "0x%"PRIx16"; %"PRIi16"\n", L1, (int16_t) L1);
        return 0;
    case INST_JMPF:
        fprintf(output, "\tJMPF %s 0x%"PRIx16"; i: %"PRIi16"\n", get_reg_str(R1, buff[0]), L2, (int16_t) L2);
        return 0;
    case INST_JMPFN:
        fprintf(output, "\tJMPFN %s 0x%"PRIx16"; i: %"PRIi16"\n", get_reg_str(R1, buff[0]), L2, (int16_t) L2);
        return 0;
    case INST_CALL:
        fprintf(output, "\tCALL ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff[0]));
        else
            fprintf(output, "0x%"PRIx16"; i: %"PRIi16"\n", L1, (int16_t) L1);
        return 0;
    case INST_RET:
        fprintf(output, "\tRET\n");
        return 0;
    case INST_ADD8:
        fprintf(output, "\tADD8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SUB8:
        fprintf(output, "\tSUB8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MUL8:
        fprintf(output, "\tMUL8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_ADD16:
        fprintf(output, "\tADD16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SUB16:
        fprintf(output, "\tSUB16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MUL16:
        fprintf(output, "\tMUL16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_ADD32:
        fprintf(output, "\tADD32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SUB32:
        fprintf(output, "\tSUB32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MUL32:
        fprintf(output, "\tMUL32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_ADD:
        fprintf(output, "\tADD %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SUB:
        fprintf(output, "\tSUB %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MUL:
        fprintf(output, "\tMUL %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_DIVI:
        fprintf(output, "\tDIVI %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_DIVU:
        fprintf(output, "\tDIVU %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_ADDF:
        fprintf(output, "\tADDF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SUBF:
        fprintf(output, "\tSUBF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MULF:
        fprintf(output, "\tMULF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_DIVF:
        fprintf(output, "\tDIVF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_INC:
        fprintf(output, "\tINC %s 0x%"PRIx16"; u: %"PRIu16"\n", get_reg_str(R1, buff[0]), L2, L2);
        return 0;
    case INST_DEC:
        fprintf(output, "\tDEC %s 0x%"PRIx16"; u: %"PRIu16"\n", get_reg_str(R1, buff[0]), L2, L2);
        return 0;
    case INST_INCF:
        fprintf(output, "\tINCF %s 0x%"PRIx16"; f: %f\n", get_reg_str(R1, buff[0]), L2, (float) L2);
        return 0;
    case INST_DECF:
        fprintf(output, "\tDECF %s 0x%"PRIx16"; f: %f\n", get_reg_str(R1, buff[0]), L2, (float) L2);
        return 0;
    case INST_ABS:
        fprintf(output, "\tABS %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_ABSF:
        fprintf(output, "\tABSF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_NEQ:
        fprintf(output, "\tNEQ %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_EQ:
        fprintf(output, "\tEQ %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_EQF:
        fprintf(output, "\tEQF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_BIGI:
        fprintf(output, "\tBIGI %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_BIGU:
        fprintf(output, "\tBIGU %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_BIGF:
        fprintf(output, "\tBIGF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SMLI:
        fprintf(output, "\tSMLI %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SMLU:
        fprintf(output, "\tSMLU %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_SMLF:
        fprintf(output, "\tSMLF %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_CASTIU:
        fprintf(output, "\tCASTIU %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CASTIF:
        fprintf(output, "\tCASTIF %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CASTUI:
        fprintf(output, "\tCASTUI %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CASTUF:
        fprintf(output, "\tCASTUF %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CASTFI:
        fprintf(output, "\tCASTFI %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CASTFU:
        fprintf(output, "\tCASTFU %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CF3264:
        fprintf(output, "\tCF3264 %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_CF6432:
        fprintf(output, "\tCF6432 %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;

    case INST_MEMSET:
        fprintf(output, "\tMEMSET %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MEMCPY:
        fprintf(output, "\tMEMCPY %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MEMMOV:
        fprintf(output, "\tMEMMOV %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MEMCMP:
        fprintf(output, "\tMEMCMP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MALLOC:
        fprintf(output, "\tMALLOC %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_FREE:
        fprintf(output, "\tFREE %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    
    case INST_FOPEN:
        fprintf(output, "\tFOPEN %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_FCLOSE:
        fprintf(output, "\tFCLOSE %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_PUTC:
        fprintf(output, "\tPUTC %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_GETC:
        fprintf(output, "\tGETC %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_FPOS:
        fprintf(output, "\tFPOS %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_FGOTO:
        fprintf(output, "\tFGOTO %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;

    case INST_FLOAT:
        fprintf(output, "\tFLOAT %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_LOAD1:{
            const Register v = (Register){
                .as_uint32 = ((uint64_t) (program[ip + 1] & 0XFFFFFF00) << 8) | ((uint64_t) (inst & 0XFFFF0000) >> 16)
            };
            fprintf(output, "\tLOAD1 %s 0x%"PRIx32"; (u: %"PRIu32"; %"PRIi32"; f: %f)\n", get_reg_str(R1, buff[0]), v.as_uint32, v.as_uint32, v.as_int32, v.as_float32);
        }
        return 0;
    case INST_LOAD2:{
            const Register v = (Register){
                .as_uint64 = (uint64_t) ((uint64_t) (program[ip + 2] & 0XFFFFFF00) << 32) |
                ((uint64_t) (program[ip + 1] & 0XFFFFFF00) << 8) | ((uint64_t) (inst & 0XFFFF0000) >> 16)
            };
            fprintf(output, "\tLOAD2 %s 0x%"PRIx64"; (u: %"PRIu64"; %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), v.as_uint64, v.as_uint64, v.as_int64, v.as_float64);
        }
        return 0;

    case INST_IOE:
        fprintf(output, "\tIOE %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_EXEC:
        fprintf(output, "\tEXEC %s\n", get_reg_str(R1, buff[0]));
        return 0;
    case INST_DISREG:
        fprintf(output, "\tDISREG %s\n", get_reg_str(R1, buff[0]));
        return 0;
    case INST_SYS:
        fprintf(output, "\tSYS ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff[0]));
        else
            fprintf(output, "%"PRIx16"\n", L1);

        return 0;

    case INST_CONTAINER:
        fprintf(output, ";;CONTAINER 0x%"PRIx32"\n", (inst & 0xFFFFFF00) >> 8);
        return 0;
    default:
        fprintf(stderr, "[ERROR] Unkonwn Instruction OpCode %u\n", inst & 0xFF);
        return 1;
    }

    #undef R1
    #undef R2
    #undef R3
    #undef L1
    #undef L2
}

int disassembler_invalid_label(const uint8_t* labels, uint64_t current_label){
    const Label label = get_label_from_raw_data(labels + current_label);
    fprintf(
        stderr,
        "[ERROR] Corrupted File: Invalid Label:\n"
        "Label:\n"
        "   size: %"PRIu32"\n"
        "   name: %s\n"
        "   type: %"PRIu8"\n",
        label.size,
        (const char*)(labels + current_label + label.str),
        label.type
    );
    if(label.type == TKN_STR)
        fprintf(stderr, "    definition: %s\n", (const char*)(labels + current_label + label.definition.as_uint + sizeof(uint64_t)));
    else
        fprintf(stderr, "    definition: %"PRIx64"\n", label.definition.as_uint);
    return 1;
}

int disassemble_handle_label(FILE* output, const void* _label, uint64_t* queried_stop){
    const Label label = get_label_from_raw_data(_label);
    const char* name = (const char*)((uint8_t*) _label) + label.str;
    switch (label.type)
    {
    case TKN_RAW:
    case TKN_STR:{
        const uint64_t str_len = *(uint64_t*)((uint8_t*)(_label) + label.definition.as_uint);
        const char* str = (char*) ((uint8_t*)(_label) + label.definition.as_uint + sizeof(uint32_t));
        fprintf(output, "%clabel %.*s %.*s\n", '%', label.str_size, name, (int) str_len, str);
    }   break;
    case TKN_ILIT:
        fprintf(output, "%clabel %.*s %"PRIi64"\n", '%', label.str_size, name, label.definition.as_int);
        break;
    case TKN_ULIT:
        fprintf(output, "%clabel %.*s %"PRIu64"\n", '%', label.str_size, name, label.definition.as_uint);
        break;
    case TKN_FLIT:
        fprintf(output, "%clabel %.*s %lf\n", '%', label.str_size, name, label.definition.as_float);
        break;
    case TKN_CHAR:
        fprintf(output, "%clabel %.*s \'%c\'\n", '%', label.str_size, name, label.definition.as_char);
        break;
    case TKN_EMPTY:
        fprintf(output, "%clabelv %.*s\n", '%', label.str_size, name);
        break;
    case TKN_INST_POSITION:
        *queried_stop = label.definition.as_uint;
        break;
    
    default:
        return 1;
    }
    return 0;
}

// disassembles program in input_path, writing the result to output_path
// \returns 0 on success or error identifier on failure
int disassemble(const char* input_path, const char* output_path){

    Mc_stream_t stream = mc_create_stream(1024);

    if(!read_file(&stream, input_path, 1, 0)){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", input_path);
        mc_destroy_stream(stream);
        return 1;
    }

    uint32_t padding;
    uint64_t flags;
    uint64_t entry_point;
    uint64_t meta_data_size;

    const uint64_t skip = sizeof(uint32_t) + sizeof(padding) + sizeof(flags) + sizeof(entry_point) + sizeof(meta_data_size);

    uint8_t* meta_data = get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags, &padding);

    if(meta_data == NULL){
        fprintf(stderr, "[ERROR] No Meta Data Found In '%s'\n", input_path);
        mc_destroy_stream(stream);
        return 1;
    }

    uint8_t* static_memory = NULL;
    uint64_t static_memory_size = 0;
    uint8_t* labels        = NULL;
    uint64_t labels_byte_size = 0;

    for(size_t i = 0; i + 8 < meta_data_size; ){
        const uint64_t size = *(uint64_t*)((uint8_t*)(meta_data) + i);
        const uint64_t id   = *(uint64_t*)((uint8_t*)(meta_data) + i + sizeof(uint64_t));
        if(size == 0){
            fprintf(stderr, "[ERROR] Corrupted File: Metadata With Block Of Size 0 (%"PRIu64")\n", id);
            mc_destroy_stream(stream);
            return 1;
        }
        else if(id == (is_little_endian()? mc_swap64(0x5354415449433a00) : 0x5354415449433a00)){
            static_memory = (uint8_t*)(meta_data) + i;
            static_memory_size = size;
        }
        else if(id == (is_little_endian()? mc_swap64(0x4c4142454c533a00) : 0x4c4142454c533a00)){
            labels = (uint8_t*)(meta_data) + i + sizeof(size) + sizeof(id);
            labels_byte_size = (size > sizeof(size) + sizeof(id))? size - sizeof(size) - sizeof(id) : 0;
        }
        i+=size;
    }

    const uint64_t inst_count = (stream.size - skip - meta_data_size - padding) / sizeof(Inst);

    FILE* output = (output_path)? fopen(output_path, "w") : stdout;

    if(!output){
        fprintf(stderr, "[ERROR] Could Not Open Output File '%s'\n", output_path);
        mc_destroy_stream(stream);
        return 1;
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
        "\t;; labels          = { position = %"PRIu64", size = %"PRIu64" }\n"
        "\t;; entry point     = %"PRIu64"\n"
        ";; X====X (SPECIFICATIONS) X====X\n\n\n",
        input_path,
        stream.size,
        inst_count,
        (uint32_t) sizeof(Inst),
        flags,
        meta_data_size,
        (uint64_t)(size_t)(static_memory - meta_data) * (!!static_memory), static_memory_size,
        (uint64_t)(size_t)(labels - meta_data) * (!!labels), labels_byte_size,
        entry_point
    );
    
    if(static_memory_size > 16){
    	fprintf(output, "%cstatic 0x", '%');
        for(uint64_t i = 16; i < static_memory_size; i+=1){
            fprintf(output, "%02"PRIx8"", static_memory[i]);
        }
        fprintf(output, "\n");
    }

    uint64_t queried_stop = inst_count;
    uint64_t last_label = 0;
    
    for(uint64_t current_label = last_label; current_label < labels_byte_size && queried_stop == inst_count; ){
        const Label label = get_label_from_raw_data(labels + current_label);
        if(label.size == 0 || disassemble_handle_label(output, labels + current_label, &queried_stop)){
            disassembler_invalid_label(labels, current_label);
            if(output_path) fclose(output);
            mc_destroy_stream(stream);
            return 1;
        }
        last_label = current_label;
        current_label += label.size;
    }

    const Inst* program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    char charbuff[30];

    char* buff[3] = {
        &charbuff[0],
        &charbuff[10],
        &charbuff[20]
    };

    int status = 0;
    uint64_t i = 0;
    
    while (queried_stop < entry_point && i < entry_point && last_label < labels_byte_size)
    {
        for(; i < queried_stop && i < inst_count && !status; i+=1){
            status = print_inst(output, program, static_memory, i, buff);
        }
        if(status || i == inst_count) break;
        const Label l = get_label_from_raw_data(labels + last_label);
        fprintf(output, "%.*s:\n", (int) l.str_size, (const char*)(labels + last_label + l.str));
        for(uint64_t current_label = last_label; current_label < labels_byte_size && i == queried_stop; ){
            const Label label = get_label_from_raw_data(labels + current_label);
            if(label.size == 0 || disassemble_handle_label(output, labels + current_label, &queried_stop)){
                disassembler_invalid_label(labels, current_label);
                if(output_path) fclose(output);
                mc_destroy_stream(stream);
                return 1;
            }
            last_label = current_label;
            current_label += label.size;
        }
        if(i == queried_stop) break;
    }
    
    for( ; (i < entry_point) && !status; i += 1)
        status = print_inst(output, program, static_memory, i, buff);
    if(!status) fprintf(output, "%s\n", "%start");

    while (i < inst_count && last_label < labels_byte_size && i < queried_stop)
    {
        for(; i < queried_stop && i < inst_count && !status; i+=1){
            status = print_inst(output, program, static_memory, i, buff);
        }
        if(status || i == inst_count) break;
        const Label label = get_label_from_raw_data(labels + last_label);
        fprintf(output, "%.*s:\n", (int) label.str_size, (const char*)(labels + last_label + label.str));
        for(uint64_t current_label = last_label; current_label < labels_byte_size && i == queried_stop; ){
            const Label label = get_label_from_raw_data(labels + current_label);
            if(label.size == 0 || disassemble_handle_label(output, labels + current_label, &queried_stop)){
                disassembler_invalid_label(labels, current_label);
                if(output_path) fclose(output);
                mc_destroy_stream(stream);
                return 1;
            }
            last_label = current_label;
            current_label += label.size;
        }
    }
    for(uint64_t current_label = last_label; current_label < labels_byte_size; ){
        const Label label = get_label_from_raw_data(labels + current_label);
        if(label.size == 0 || disassemble_handle_label(output, labels + current_label, &queried_stop)){
            disassembler_invalid_label(labels, current_label);
            if(output_path) fclose(output);
            mc_destroy_stream(stream);
            return 1;
        }
        last_label = current_label;
        current_label += label.size;
    }

    for( ; (i < inst_count) && !status; i += 1)
        status = print_inst(output, program, static_memory, i, buff);
    

    if(output_path) fclose(output);
    if(status) fprintf(stderr, "[ERROR] At Instruction Position %" PRIu64 " ^^^\n", i);
    mc_destroy_stream(stream);
    return status;
}



#endif // END _OF FILE =============================================
