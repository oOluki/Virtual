#ifndef _VPU_DISASSEMBLER
#define _VPU_DISASSEMBLER

#include "core.h"
#include "lexer.h"
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include "labels.h"
#include "virtual_files.h"
#include "virtual.h"


// \param buff should be an array of 3 buffers of size 8 bytes each
// \returns 0 on success or 1 otherwise
int print_inst(FILE* output, Inst inst, char** buff){
    #define R1 (uint8_t) ((inst & 0XFF00) >> 8)
    #define R2 (uint8_t) ((inst & 0XFF0000) >> 16)
    #define R3 (uint8_t) (inst >> 24)
    #define L2 (uint16_t) (inst >> 16)
    #define L1 (uint16_t) ((inst & 0X00FFFF00) >> 8)


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
    case INST_STACK_GET:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tSTACK_GET %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_STACK_PUT:{
        Register op = {.as_uint64 = L2};
        fprintf(output, "\tSTACK_PUT %s 0x%02"PRIx64"; (u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff[0]), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_GSP:
        fprintf(output, "\tGSP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_STATIC:
        if(GET_OP_HINT(inst) == HINT_REG)
	        fprintf(output, "\tSTATIC %s\n", get_reg_str(R1, buff[0]));
        else
            fprintf(output, "\tSTATIC 0x%"PRIx16"\n", L1);
        return 0;
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
    case INST_MREADS:
        fprintf(output, "\tMREADS %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_WRITE8:
        fprintf(output, "\tWRITE8 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_WRITE16:
        fprintf(output, "\tWRITE16 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_WRITE32:
        fprintf(output, "\tWRITE32 %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_WRITE:
        fprintf(output, "\tWRITE %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MWRITES:
        fprintf(output, "\tMWRITES %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MMOVS:
        fprintf(output, "\tMMOVS %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_MEMCMP:
        fprintf(output, "\tMEMCMP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
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

    case INST_FLOAT:
        fprintf(output, "\tFLOAT %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_DUMPCHAR:
        fprintf(output, "\tDUMPCHAR %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
    case INST_GETCHAR:
        fprintf(output, "\tGETCHAR %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]));
        return 0;
    case INST_EXEC:
        fprintf(output, "\tEXEC %s\n", get_reg_str(R1, buff[0]));
        return 0;
    case INST_SYS:
        fprintf(output, "\tSYS ");
        if(GET_OP_HINT(inst) == HINT_REG)
            fprintf(output, "%s\n", get_reg_str(R1, buff[0]));
        else
            fprintf(output, "%"PRIu16"\n", L1);

        return 0;
    case INST_DISREG:
        fprintf(output, "\tDISREG %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
	case INST_GRP:
	    fprintf(output, "\tGRP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;
	case INST_GIP:
	    fprintf(output, "\tGIP %s %s %s\n", get_reg_str(R1, buff[0]), get_reg_str(R2, buff[1]), get_reg_str(R3, buff[2]));
        return 0;

    case INST_CONTAINER:
        fprintf(output, "\tCONTAINER 0x%"PRIx32"\n", (inst & 0xFFFFFF00) >> 8);
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
int disassemble(
    FILE* output,
    const Inst* program, uint64_t inst_count, uint64_t entry_point,
    const void* labels, uint64_t labels_byte_size,
    const void* static_memory, uint64_t static_memory_size
){

    /*fprintf( output,
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
    );*/
    
    if(static_memory && static_memory_size){
    	fprintf(output, "%cstatic 0x", '%');
        for(uint64_t i = 0; i < static_memory_size; i+=1){
            fprintf(output, "%02"PRIx8"", ((uint8_t*) static_memory)[i]);
        }
        fprintf(output, "\n\n");
    }

    uint64_t queried_stop = inst_count;
    uint64_t last_label = 0;
    
    for(uint64_t current_label = last_label; current_label < labels_byte_size && queried_stop == inst_count; ){
        const Label label = get_label_from_raw_data(((uint8_t*) labels) + current_label);
        if(label.size == 0 || disassemble_handle_label(output, ((uint8_t*) labels) + current_label, &queried_stop)){
            disassembler_invalid_label((uint8_t*) labels, current_label);
            return 1;
        }
        last_label = current_label;
        current_label += label.size;
    }

    char charbuff[24];

    char* buff[3] = {
        &charbuff[0],
        &charbuff[8],
        &charbuff[16]
    };

    int status = 0;
    uint64_t i = 0;
    
    while (queried_stop < entry_point && i < entry_point && last_label < labels_byte_size)
    {
        for(; i < queried_stop && i < inst_count && !status; i+=1){
            status = print_inst(output, program[i], buff);
        }
        if(status || i == inst_count) break;
        const Label l = get_label_from_raw_data(((uint8_t*) labels) + last_label);
        fprintf(output, "%.*s:\n", (int) l.str_size, (const char*)(((uint8_t*) labels) + last_label + l.str));
        for(uint64_t current_label = last_label; current_label < labels_byte_size && i == queried_stop; ){
            const Label label = get_label_from_raw_data(((uint8_t*) labels) + current_label);
            if(label.size == 0 || disassemble_handle_label(output, ((uint8_t*) labels) + current_label, &queried_stop)){
                disassembler_invalid_label(((uint8_t*) labels), current_label);
                return 1;
            }
            last_label = current_label;
            current_label += label.size;
        }
        if(i == queried_stop) break;
    }
    
    for( ; (i < entry_point) && !status; i += 1)
        status = print_inst(output, program[i], buff);
    if(!status) fprintf(output, "%s\n", "%start");

    while (i < inst_count && last_label < labels_byte_size && i < queried_stop)
    {
        for(; i < queried_stop && i < inst_count && !status; i+=1){
            status = print_inst(output, program[i], buff);
        }
        if(status || i == inst_count) break;
        const Label label = get_label_from_raw_data(((uint8_t*) labels) + last_label);
        fprintf(output, "%.*s:\n", (int) label.str_size, (const char*)(((uint8_t*) labels) + last_label + label.str));
        for(uint64_t current_label = last_label; current_label < labels_byte_size && i == queried_stop; ){
            const Label label = get_label_from_raw_data(((uint8_t*) labels) + current_label);
            if(label.size == 0 || disassemble_handle_label(output, ((uint8_t*) labels) + current_label, &queried_stop)){
                disassembler_invalid_label((uint8_t*) labels, current_label);
                return 1;
            }
            last_label = current_label;
            current_label += label.size;
        }
    }
    for(uint64_t current_label = last_label; current_label < labels_byte_size; ){
        const Label label = get_label_from_raw_data(((uint8_t*) labels) + current_label);
        if(label.size == 0 || disassemble_handle_label(output, ((uint8_t*) labels) + current_label, &queried_stop)){
            disassembler_invalid_label((uint8_t*) labels, current_label);
            return 1;
        }
        last_label = current_label;
        current_label += label.size;
    }

    for( ; (i < inst_count) && !status; i += 1)
        status = print_inst(output, program[i], buff);
    

    if(status) fprintf(stderr, "[ERROR] At Instruction Position %" PRIu64 " ^^^\n", i);
    return status;
}


int disassemble_file(const char* input_file, const char* output_file){

    VirtualFile vfile;
    const char* required_fields[] = {
        VIRTUAL_FILE_PROGRAM_FIELD_NAME,
        NULL
    };
    const char* optional_fields[] = {
        VIRTUAL_FILE_LABELS_FIELD_NAME,
        VIRTUAL_FILE_STATIC_FIELD_NAME,
        NULL
    };
    if(vfopen(&vfile, input_file, required_fields, optional_fields)){
        fprintf(stderr, "[ERROR] disassembler failed trying to open virtual file '%s'\n", input_file);
        return 1;
    }

    const void* const program_field = get_virtual_file_field(vfile, VIRTUAL_FILE_PROGRAM_FIELD_NAME);
    
    uint64_t entry_point;
    uint64_t program_size;
    const Inst* program = get_program_from_vfield(program_field, &program_size, &entry_point);
    if(program == NULL){
        fprintf(stderr, "[ERROR] disassembler failed, virtual file in '%s' has corrupt program\n", input_file);
        vfclose(vfile);
        return 1;
    }

    const void* labels = get_virtual_file_field(vfile, VIRTUAL_FILE_LABELS_FIELD_NAME);
    uint64_t labels_size = 0;
    if(labels){
        labels_size = *(uint64_t*) labels;
        labels_size -= sizeof(uint64_t) + sizeof(VIRTUAL_FILE_LABELS_FIELD_NAME);
        labels = (void*) (((uintptr_t) labels) + sizeof(uint64_t) + sizeof(VIRTUAL_FILE_LABELS_FIELD_NAME));
    }

    const void* static_memory = get_virtual_file_field(vfile, VIRTUAL_FILE_STATIC_FIELD_NAME);
    uint64_t static_memory_size = 0;
    if(static_memory){
        static_memory_size = *(uint64_t*) static_memory;
        static_memory_size -= sizeof(static_memory_size) + sizeof(VIRTUAL_FILE_STATIC_FIELD_NAME);
        static_memory = (void*) (((uintptr_t) static_memory) + sizeof(static_memory_size) + sizeof(VIRTUAL_FILE_STATIC_FIELD_NAME));
    }

    FILE* output = output_file? fopen(output_file, "w") : stdout;
    if(!output){
        fprintf(stderr, "[ERROR] disssembler could not open output file '%s'\n", output_file);
        vfclose(vfile);
        return 1;
    }

    if(disassemble(output, program, program_size, entry_point, labels, labels_size, static_memory, static_memory_size)){
        fprintf(stderr, "[ERROR] disassembler failed to disassemble program at '%s'\n", input_file);
        vfclose(vfile);
        if(output && output != stdout) fclose(output);
        return 1;
    }
    vfclose(vfile);
    if(output && output != stdout) fclose(output);
    return 0;
}


#endif // END _OF FILE =============================================
