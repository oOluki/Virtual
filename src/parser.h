#ifndef VIRTUAL_PARSER_H
#define VIRTUAL_PARSER_H

#include "lexer.h"
#include <stdio.h>
#include "core.h"
#include "labels.h"
#include <inttypes.h>
#include <stdarg.h>

#define COMP_TKN(TKN1, TKN2) mc_compare_token(TKN1, TKN2, 0)

#define REPORT_ERROR(PARSER, STR, ...) fprintf(\
        stderr,\
        "[ERROR] %s:%i:%i :  " STR,\
        PARSER->file_path, PARSER->tokenizer->line + 1,\
        PARSER->tokenizer->column + 1, __VA_ARGS__\
    )

#define REP_INVALID_REGID(PARSER, TOKEN) REPORT_ERROR(PARSER, "\n\tInvalid Register Identifier '%.*s', No Such Register\n", TOKEN.size, TOKEN.value.as_str)

typedef struct Operand{
    
    Register value;
    uint8_t type;

} Operand;

typedef struct InstProfile{

    uint8_t   opcode;
    OpProfile op_profile;

} InstProfile;

typedef struct Parser
{
    char* file_path;
    int file_path_size;
    Mc_stream_t* labels;
    Mc_stream_t* local_labels;
    Mc_stream_t* static_memory;
    Mc_stream_t* program;
    Tokenizer* tokenizer;
    uint32_t flags;
    int macro_if_depth;
    uint64_t entry_point;
} Parser;


void fprint_token(FILE* file, const Token token){
    switch(token.type){
    case TKN_NONE:
	fprintf(file, "TOKEN_NONE");
	break;
    case TKN_RAW:
    case TKN_INST:
    case TKN_REG:
    case TKN_STR:
    case TKN_CHAR:
    case TKN_MACRO_INST:
    case TKN_LABEL_REF:
    case TKN_ADDR_LABEL_REF:
        fprintf(file, "%.*s", token.size, token.value.as_str);
        break;
    case TKN_ILIT:
        fprintf(file, "%"PRIi64"", token.value.as_int);
        break;
    case TKN_INST_POSITION:
    case TKN_ULIT:
        fprintf(file, "%"PRIu64"", token.value.as_uint);
        break;
    case TKN_FLIT:
        fprintf(file, "%f", token.value.as_float);
        break;
    case TKN_SPECIAL_SYM:
        fprintf(file, "%c", token.value.as_char);
        break;
    case TKN_EMPTY:
        fprintf(file, "TOKEN_EMPTY");
        break;
    case TKN_ERROR:
    default:
        fprintf(file, "TOKEN_ERROR(%"PRIu8")", token.type);
        break;
    }

}

const char* get_token_type_str(int type){

    switch (type)
    {
    case TKN_NONE:              return "TOKEN_NONE";
    case TKN_RAW:               return "TOKEN_RAW";
    case TKN_INST:              return "TOKEN_INST";
    case TKN_REG:               return "TOKEN_REG";
    case TKN_NUM:               return "TOKEN_NUM";
    case TKN_ILIT:              return "TOKEN_ILIT";
    case TKN_ULIT:              return "TOKEN_ULIT";
    case TKN_FLIT:              return "TOKEN_FLIT";
    case TKN_STR:               return "TOKEN_STR";
    case TKN_CHAR:              return "TOKEN_CHAR";
    case TKN_SPECIAL_SYM:       return "TOKEN_SPECIAL_SYM";
    case TKN_MACRO_INST:        return "TOKEN_MACRO_INST";
    case TKN_LABEL_REF:         return "TOKEN_LABEL_REF";
    case TKN_EMPTY:             return "TOKEN_EMPTY";
    case TKN_ADDR_LABEL_REF:    return "TOKEN_ADDR_LABEL_REF";
    case TKN_STATIC_SIZE:       return "TOKEN_STATIC_SIZE";
    case TKN_UNRESOLVED_LABEL:  return "TKN_UNRESOLVED_LABEL";
    case TKN_INST_POSITION:     return "TKN_INST_POSITION";
    default:                    return "TOKEN_ERROR";
    }

}

InstProfile get_inst_profile(const Token inst_token){
    #define __VIRTUAL_RETURN_INST_PROFILE(INST_NAME, OP_PROFILE, ...)\
        if(COMP_TKN(inst_token, MKTKN(#INST_NAME))) return (InstProfile){INST_ ## INST_NAME, OP_PROFILE}

    if(inst_token.type != TKN_RAW)            return (InstProfile){INST_ERROR , 0};

    __VIRTUAL_RETURN_INST_PROFILE(NOP,       OP_PROFILE_NONE, INST_NOP  );
    __VIRTUAL_RETURN_INST_PROFILE(HALT,      OP_PROFILE_E  , INST_HALT  );
    __VIRTUAL_RETURN_INST_PROFILE(MOV8,      OP_PROFILE_RR , INST_MOV8  );
    __VIRTUAL_RETURN_INST_PROFILE(MOV16,     OP_PROFILE_RR , INST_MOV16 );
    __VIRTUAL_RETURN_INST_PROFILE(MOV32,     OP_PROFILE_RR , INST_MOV32 );
    __VIRTUAL_RETURN_INST_PROFILE(MOV,       OP_PROFILE_RR , INST_MOV   );
    __VIRTUAL_RETURN_INST_PROFILE(MOVC,      OP_PROFILE_RRR, INST_MOVC  );
    __VIRTUAL_RETURN_INST_PROFILE(MOVV,      OP_PROFILE_RL , INST_MOVV  );
    __VIRTUAL_RETURN_INST_PROFILE(MOVN,      OP_PROFILE_RL , INST_MOVN  );
    __VIRTUAL_RETURN_INST_PROFILE(MOVV16,    OP_PROFILE_RL , INST_MOVV16);
    __VIRTUAL_RETURN_INST_PROFILE(PUSH,      OP_PROFILE_E  , INST_PUSH  );
    __VIRTUAL_RETURN_INST_PROFILE(POP,       OP_PROFILE_R  , INST_POP   );
    __VIRTUAL_RETURN_INST_PROFILE(STACK_GET, OP_PROFILE_RL , INST_STACK_GET);
    __VIRTUAL_RETURN_INST_PROFILE(STACK_PUT, OP_PROFILE_RL , INST_STACK_PUT);
    __VIRTUAL_RETURN_INST_PROFILE(GSP,       OP_PROFILE_RRR, INST_GSP   );
    __VIRTUAL_RETURN_INST_PROFILE(STATIC,    OP_PROFILE_E  , INST_STATIC);
    __VIRTUAL_RETURN_INST_PROFILE(READ8,     OP_PROFILE_RRR, INST_READ8 );
    __VIRTUAL_RETURN_INST_PROFILE(READ16,    OP_PROFILE_RRR, INST_READ16);
    __VIRTUAL_RETURN_INST_PROFILE(READ32,    OP_PROFILE_RRR, INST_READ32);
    __VIRTUAL_RETURN_INST_PROFILE(READ,      OP_PROFILE_RRR, INST_READ  );
    __VIRTUAL_RETURN_INST_PROFILE(MREADS,    OP_PROFILE_RRR, INST_MREADS);
    __VIRTUAL_RETURN_INST_PROFILE(WRITE8,    OP_PROFILE_RRR, INST_WRITE8 );
    __VIRTUAL_RETURN_INST_PROFILE(WRITE16,   OP_PROFILE_RRR, INST_WRITE16);
    __VIRTUAL_RETURN_INST_PROFILE(WRITE32,   OP_PROFILE_RRR, INST_WRITE32);
    __VIRTUAL_RETURN_INST_PROFILE(WRITE,     OP_PROFILE_RRR, INST_WRITE  );
    __VIRTUAL_RETURN_INST_PROFILE(MWRITES,   OP_PROFILE_RRR, INST_MWRITES);
    __VIRTUAL_RETURN_INST_PROFILE(MMOVS,     OP_PROFILE_RRR, INST_MMOVS );
    __VIRTUAL_RETURN_INST_PROFILE(MEMCMP,    OP_PROFILE_RRR, INST_MEMCMP);
    __VIRTUAL_RETURN_INST_PROFILE(NOT,       OP_PROFILE_RR , INST_NOT   );
    __VIRTUAL_RETURN_INST_PROFILE(NEG,       OP_PROFILE_RRR, INST_NEG   );
    __VIRTUAL_RETURN_INST_PROFILE(AND,       OP_PROFILE_RRR, INST_AND   );
    __VIRTUAL_RETURN_INST_PROFILE(NAND,      OP_PROFILE_RRR, INST_NAND  );
    __VIRTUAL_RETURN_INST_PROFILE(OR,        OP_PROFILE_RRR, INST_OR    );
    __VIRTUAL_RETURN_INST_PROFILE(XOR,       OP_PROFILE_RRR, INST_XOR   );
    __VIRTUAL_RETURN_INST_PROFILE(BSHIFT,    OP_PROFILE_RRR, INST_BSHIFT);
    __VIRTUAL_RETURN_INST_PROFILE(JMP,       OP_PROFILE_E  , INST_JMP   );
    __VIRTUAL_RETURN_INST_PROFILE(JMPF,      OP_PROFILE_RL , INST_JMPF  );
    __VIRTUAL_RETURN_INST_PROFILE(JMPFN,     OP_PROFILE_RL , INST_JMPFN );
    __VIRTUAL_RETURN_INST_PROFILE(CALL,      OP_PROFILE_E  , INST_CALL  );
    __VIRTUAL_RETURN_INST_PROFILE(RET,       OP_PROFILE_NONE, INST_RET  );
    __VIRTUAL_RETURN_INST_PROFILE(ADD8,      OP_PROFILE_RRR, INST_ADD8  );
    __VIRTUAL_RETURN_INST_PROFILE(SUB8,      OP_PROFILE_RRR, INST_SUB8  );
    __VIRTUAL_RETURN_INST_PROFILE(MUL8,      OP_PROFILE_RRR, INST_MUL8  );
    __VIRTUAL_RETURN_INST_PROFILE(ADD16,     OP_PROFILE_RRR, INST_ADD16 );
    __VIRTUAL_RETURN_INST_PROFILE(SUB16,     OP_PROFILE_RRR, INST_SUB16 );
    __VIRTUAL_RETURN_INST_PROFILE(MUL16,     OP_PROFILE_RRR, INST_MUL16 );
    __VIRTUAL_RETURN_INST_PROFILE(ADD32,     OP_PROFILE_RRR, INST_ADD32 );
    __VIRTUAL_RETURN_INST_PROFILE(SUB32,     OP_PROFILE_RRR, INST_SUB32 );
    __VIRTUAL_RETURN_INST_PROFILE(MUL32,     OP_PROFILE_RRR, INST_MUL32 );
    __VIRTUAL_RETURN_INST_PROFILE(ADD,       OP_PROFILE_RRR, INST_ADD   );
    __VIRTUAL_RETURN_INST_PROFILE(SUB,       OP_PROFILE_RRR, INST_SUB   );
    __VIRTUAL_RETURN_INST_PROFILE(MUL,       OP_PROFILE_RRR, INST_MUL   );
    __VIRTUAL_RETURN_INST_PROFILE(DIVI,      OP_PROFILE_RRR, INST_DIVI  );
    __VIRTUAL_RETURN_INST_PROFILE(DIVU,      OP_PROFILE_RRR, INST_DIVU  );
    __VIRTUAL_RETURN_INST_PROFILE(ADDF,      OP_PROFILE_RRR, INST_ADDF  );
    __VIRTUAL_RETURN_INST_PROFILE(SUBF,      OP_PROFILE_RRR, INST_SUBF  );
    __VIRTUAL_RETURN_INST_PROFILE(MULF,      OP_PROFILE_RRR, INST_MULF  );
    __VIRTUAL_RETURN_INST_PROFILE(DIVF,      OP_PROFILE_RRR, INST_DIVF  );
    __VIRTUAL_RETURN_INST_PROFILE(INC,       OP_PROFILE_RL , INST_INC   );
    __VIRTUAL_RETURN_INST_PROFILE(DEC,       OP_PROFILE_RL , INST_DEC   );
    __VIRTUAL_RETURN_INST_PROFILE(INCF,      OP_PROFILE_RL , INST_INCF  );
    __VIRTUAL_RETURN_INST_PROFILE(DECF,      OP_PROFILE_RL , INST_DECF  );
    __VIRTUAL_RETURN_INST_PROFILE(ABS,       OP_PROFILE_RRR, INST_ABS   );
    __VIRTUAL_RETURN_INST_PROFILE(ABSF,      OP_PROFILE_RRR, INST_ABSF  );
    __VIRTUAL_RETURN_INST_PROFILE(NEQ,       OP_PROFILE_RRR, INST_NEQ   );
    __VIRTUAL_RETURN_INST_PROFILE(EQ,        OP_PROFILE_RRR, INST_EQ    );
    __VIRTUAL_RETURN_INST_PROFILE(EQF,       OP_PROFILE_RRR, INST_EQF   );
    __VIRTUAL_RETURN_INST_PROFILE(BIGI,      OP_PROFILE_RRR, INST_BIGI  );
    __VIRTUAL_RETURN_INST_PROFILE(BIGU,      OP_PROFILE_RRR, INST_BIGU  );
    __VIRTUAL_RETURN_INST_PROFILE(BIGF,      OP_PROFILE_RRR, INST_BIGF  );
    __VIRTUAL_RETURN_INST_PROFILE(SMLI,      OP_PROFILE_RRR, INST_SMLI  );
    __VIRTUAL_RETURN_INST_PROFILE(SMLU,      OP_PROFILE_RRR, INST_SMLU  );
    __VIRTUAL_RETURN_INST_PROFILE(SMLF,      OP_PROFILE_RRR, INST_SMLF  );
    __VIRTUAL_RETURN_INST_PROFILE(CASTIU,    OP_PROFILE_RR , INST_CASTIU);
    __VIRTUAL_RETURN_INST_PROFILE(CASTIF,    OP_PROFILE_RR , INST_CASTIF);
    __VIRTUAL_RETURN_INST_PROFILE(CASTUI,    OP_PROFILE_RR , INST_CASTUI);
    __VIRTUAL_RETURN_INST_PROFILE(CASTUF,    OP_PROFILE_RR , INST_CASTUF);
    __VIRTUAL_RETURN_INST_PROFILE(CASTFI,    OP_PROFILE_RR , INST_CASTFI);
    __VIRTUAL_RETURN_INST_PROFILE(CASTFU,    OP_PROFILE_RR , INST_CASTFU);
    __VIRTUAL_RETURN_INST_PROFILE(CF3264,    OP_PROFILE_RR , INST_CF3264);
    __VIRTUAL_RETURN_INST_PROFILE(CF6432,    OP_PROFILE_RR , INST_CF6432);
    __VIRTUAL_RETURN_INST_PROFILE(FLOAT,     OP_PROFILE_RRR, INST_FLOAT );
    __VIRTUAL_RETURN_INST_PROFILE(DUMPCHAR,  OP_PROFILE_RRR, INST_DUMPCHAR);
    __VIRTUAL_RETURN_INST_PROFILE(GETCHAR,   OP_PROFILE_RR , INST_GETCHAR);
    __VIRTUAL_RETURN_INST_PROFILE(EXEC,      OP_PROFILE_R  , INST_EXEC  );
    __VIRTUAL_RETURN_INST_PROFILE(SYS,       OP_PROFILE_E  , INST_SYS   );
    __VIRTUAL_RETURN_INST_PROFILE(DISREG,    OP_PROFILE_RRR, INST_DISREG);
    __VIRTUAL_RETURN_INST_PROFILE(GRP,       OP_PROFILE_RRR, INST_GRP);
    __VIRTUAL_RETURN_INST_PROFILE(GIP,       OP_PROFILE_RRR, INST_GIP);

    return (InstProfile){INST_ERROR, OP_PROFILE_NONE};
    #undef __VIRTUAL_RETURN_INST_PROFILE
}

static inline int get_major_reg_identifier(const Token token){
    if(token.size < 2) return -1;
    if(token.value.as_str[0] != 'r' && token.value.as_str[0] != 'R')
        return -1;
    
    if(mc_compare_token(token, MKTKN("r0"), 1))  return MR0;
    if(mc_compare_token(token, MKTKN("R0"), 1))  return MR0;
    if(mc_compare_token(token, MKTKN("rsp"), 1) && token.size > 2) return MRSP;
    if(mc_compare_token(token, MKTKN("RSP"), 1) && token.size > 2) return MRSP;
    if(mc_compare_token(token, MKTKN("rip"), 1) && token.size > 2) return MRIP;
    if(mc_compare_token(token, MKTKN("RIP"), 1) && token.size > 2) return MRIP;

    if(token.size > 3) return -1;

    if(token.value.as_str[1] >= 'a' && token.value.as_str[1] <= 'z')
        return MRA + (token.value.as_str[1] - 'a');

    if(token.value.as_str[1] >= 'A' && token.value.as_str[1] <= 'Z')
        return MRA + (token.value.as_str[1] - 'A');

    return -1;
}

int get_reg(const Token token){
    if(token.type == TKN_REG) return (int)token.value.as_uint;
    if(token.type != TKN_RAW) return -1;
	if(token.size > 4 || token.size < 2) return -1;

    const int reg = get_major_reg_identifier(token);
    if(reg < 0) return -1;
    if(token.size == 2){
        return reg * 8;
    }
	if(token.size == 4){
        const int i = get_digit(token.value.as_str[3]);
        if((i < 0) || (i > 7)){
            return -1;
        }
        return reg * 8 + i;
	}
    if(reg == MRSP || reg == MRIP || reg == MRST)
        return reg * 8;
	if(token.size == 3){
        const int i = get_digit(token.value.as_str[2]);
        if((i < 0) || (i > 7)){
            return -1;
        }
        return reg * 8 + i;
    }

    return -1;
}

Operand parse_op_literal(Token token){
    
    switch (token.type)
    {
    case TKN_RAW:
        break;
    case TKN_ILIT:
    case TKN_ULIT:
    case TKN_FLIT:
    case TKN_INST_POSITION:
        return (Operand){.value.as_uint64 = token.value.as_uint, .type = TKN_ULIT};
    case TKN_CHAR:
        return (Operand){.value.as_uint64 = token.value.as_str[1], .type = TKN_ULIT};
    default: // unsupported type for operand literal
        return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
    }

    if(token.value.as_str == NULL) return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};


    if(token.value.as_str[0] == '0' && token.size > 1){
        if(token.value.as_str[1] == 'x' || token.value.as_str[1] == 'X'){
            uint64_t hex = 0;
            for(size_t i = 2; i < token.size; i+=1){
                const uint8_t digit = get_hex_digit(token.value.as_str[i]);

                if(digit == 255){
                    return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
                }

                hex = (hex << 4) | digit;
            }
            return (Operand){.value.as_uint64 = hex, .type = TKN_ULIT};
        }
    }

    const int is_negative = (token.value.as_str[0] == '-');
    uint64_t _10n1 = 1;
    uint64_t _10n2 = 1;
    uint64_t first_part = 0;
    uint64_t second_part = 0;
    int dot_position = -1;
    const int float_identifier =
        (token.value.as_str[token.size - 1] == 'f') ||
        (token.value.as_str[token.size - 1] == 'F');
    
    for(int i = token.size - 1 - float_identifier; i > is_negative - 1; i-=1){

        const int digit = get_digit(token.value.as_str[i]);

        if(digit < 0){
            if(token.value.as_str[i] == '.'){
                dot_position = i;
                break;
            }
            return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
        }

        first_part += digit * _10n1;
        _10n1 *= 10;
    }

    for(int i = dot_position - 1; i > is_negative - 1; i-= 1){
        const int digit = get_digit(token.value.as_str[i]);

        if(digit < 0){
            return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
        }

        second_part += digit * _10n2;
        _10n2 *= 10;
    }

    if((dot_position < 0) && !float_identifier){
        return is_negative?
            (Operand){.value.as_int64  = -(int64_t)(first_part), .type = TKN_ILIT}:
            (Operand){.value.as_uint64 =            first_part , .type = TKN_ULIT};
    }

    const double output = (double)(second_part) + (double)(first_part) / (double)(_10n1);
    
    return (Operand){.value.as_float64 = is_negative? -output : output, .type = TKN_FLIT};
}

int push_to_static(Mc_stream_t* static_memory, const Token token){

    if(token.type != TKN_STR && token.type != TKN_RAW){
        return 1;
    }
    if(token.type == TKN_STR){
        if(token.size < 2 || token.value.as_str[token.size - 1] != '\"'){
            return 2;
        }
	    mc_stream(static_memory, token.value.as_str + 1, token.size - 2);
        mc_stream_str(static_memory, "");
        return 0;
    }
    if(token.size >= 2){
        if(token.value.as_str[0] == '0' && (token.value.as_str[1] == 'x' || token.value.as_str[1] == 'X')){
            for(int i = 2; i < token.size; i+=2){

                uint8_t byte = 0;

                for(int j = 0; (j + i) < token.size && j < 2; j+=1){
                    const int digit = get_hex_digit(token.value.as_str[i + j]);
                    if(digit == 255){
                        return 1;
                    }
                    byte |= digit << (4 - 
                    j * 4);
                }
                
                mc_stream(static_memory, &byte, sizeof(byte));
            }
            return 0;
        }
    }

    const Operand v = parse_op_literal(token);
    if(v.type == TKN_ERROR){
        return 1;
    }

    mc_stream(static_memory, &v.value.as_uint64, 8);
    return 0;
}

int parse_macro(Parser* parser, const Token macro, StringView* include_path){
    
    if(COMP_TKN(macro, MKTKN("%include"))){
        const Token arg = get_next_token(parser->tokenizer);
        if(arg.type != TKN_STR){
            if(arg.type == TKN_RAW) REPORT_ERROR(
                parser,
                "\n\tIncorrect Usage For %s Macro, Expected String Got '%.*s' Instead\n\n",
                "%include", macro.size, macro.value.as_str
            );
            else REPORT_ERROR(
                parser,
                "\n\tIncorrect Usage For %s Macro, Expected String\n\n", "%include"
            );
            return 1;
        }
        if(arg.value.as_str[arg.size - 1] != '\"' || arg.size == 1){
            REPORT_ERROR(parser, "\n\tMissing Closing %c\n\n", '\"');
            return 1;
        }
        if(arg.size < 3){
            REPORT_ERROR(parser, "\n\tExpected Valid File Path String Literal, Got Empty String Instead%c\n\n", ' ');
            return 1;
        }
        *include_path = (StringView){.str = arg.value.as_str + 1, .size = arg.size - 2};
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%label"))){
        const Token arg1 = get_next_token(parser->tokenizer);
        Token arg2 = get_next_token(parser->tokenizer);
        const Token original_arg2 = arg2;

        if(arg1.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        if(arg2.type == TKN_RAW){
            const Operand op = parse_op_literal(arg2);
            if(op.type != TKN_ERROR){
                arg2.type = op.type;
                arg2.value.as_uint = op.value.as_uint64;
            }
        }
        else if(arg2.type == TKN_LABEL_REF){
            arg2 = resolve_token(parser->labels, arg2);
            if(arg2.type == TKN_ERROR){
                REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", arg2.size, arg2.value.as_str);
                return 1;
            }
        }
        else if(arg2.type == TKN_ADDR_LABEL_REF){
            arg2.type = TKN_LABEL_REF;
            arg2 = resolve_token(parser->labels, arg2);
            if(arg2.type == TKN_ERROR){
                REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", arg2.size, arg2.value.as_str);
                return 1;
            }
            if(arg2.type != TKN_ULIT){
                REPORT_ERROR(parser, "\n\tInvalid Value For Relative Referencing %c", '\'');
                fprint_token(stderr, arg2);
                fprintf(stderr, "\'\n\n");
                return 1;
            }
            arg2.value.as_int -= (parser->program->size / 4);
        }
        else if(arg2.type == TKN_STR){
            if(!parser->static_memory){
                REPORT_ERROR(
                    parser,
                    "\n\tCan not add '%.*s' label definition string, no static memory available for string allocation\n",
                    arg1.size, arg1.value.as_str
                );
                return 1;
            }
            const uint64_t spos = parser->static_memory->size;
            push_to_static(parser->static_memory, arg2);
            arg2.value.as_uint = spos;
            arg2.type = TKN_ULIT;
        }
        else if(arg2.type == TKN_NONE){
            REPORT_ERROR(parser, "\n\tMissing Definition For '%.*s' Label\n\n", arg1.size, arg1.value.as_str);
            return 1;
        }
        if(add_label(parser->labels, arg1, arg2)){
            REPORT_ERROR(
                parser, "\n\tInvalid Label Or Definition '%s %.*s %.*s'\n\tNOT: You Can Not Redifine Already Labeled Labels\n\n",
                "%label",
                arg1.size, arg1.value.as_str, original_arg2.size, original_arg2.value.as_str
            );
            return 1;
        }
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%labelv"))){
        const Token arg1 = get_next_token(parser->tokenizer);
        if(arg1.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        if(add_label(parser->labels, arg1, (Token){.type = TKN_EMPTY})){
            REPORT_ERROR(
                parser, "\n\tInvalid Label Or Definition '%s %.*s'\n\tNOTE: You Can Not Relabel\n\n",
                "%label", arg1.size, arg1.value.as_str
            );
            return 1;
        }
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%unlabel"))){
        const Token arg = get_next_token(parser->tokenizer);
        if(arg.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        if(remove_label(parser->labels, arg)){
            REPORT_ERROR(parser, "\n\tAttempting To Unlabel '%.*s' While Label Does Not Exist\n\n", arg.size, arg.value.as_str);
            return 1;
        }
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%iflabel"))){
        const Token arg = get_next_token(parser->tokenizer);
        if(arg.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        if(!get_label(parser->labels, arg)){
            tokenizer_goto(parser->tokenizer, "%endif");
            get_next_token(parser->tokenizer);
        }
        parser->macro_if_depth += 1;
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%ifnlabel"))){
        const Token arg = get_next_token(parser->tokenizer);
        if(arg.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        if(get_label(parser->labels, arg)){
            tokenizer_goto(parser->tokenizer, "%endif");
            get_next_token(parser->tokenizer);
        }
        parser->macro_if_depth += 1;
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%static"))){
        if(!parser->static_memory){
            REPORT_ERROR(parser, "\n\tNo Static Memory Available%c\n", '\n');
            return 1;
        }
        const Token arg = get_next_token(parser->tokenizer);
        const int status = push_to_static(parser->static_memory, arg);
        if(status == 2){
            REPORT_ERROR(parser, "\n\tMissing Closing %c\n\n", '\"');
            return 1;
        }
        if(status){
            if(arg.type == TKN_RAW)
                REPORT_ERROR(parser, "\n\tArgument Of %s Should Be Literal, Got %.*s Instead\n\n", "%static", arg.size, arg.value.as_str);
            else
                REPORT_ERROR(parser, "\n\tArgument Of %s Should Be Literal\n\n", "%static");
            return 1;
        }
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%enum"))){
        Token token = get_next_token(parser->tokenizer);
        Token definition = (Token){.type = TKN_ILIT, .size = 0, .value.as_int = 0};
        while(token.type != TKN_NONE && token.type != TKN_ERROR && !COMP_TKN(token, MKTKN("%endenum"))){
            if(token.type != TKN_RAW){
                REPORT_ERROR(
                    parser, "\n\t%.*s Macro Element Should Be Raw Token, Got '%.*s' Instead\n",
                    macro.size, macro.value.as_str,
                    token.size, token.value.as_str
                );
                return 1;
            }
            Token next = get_next_token(parser->tokenizer);

            if(next.type == TKN_SPECIAL_SYM && next.value.as_char == '='){
                definition = get_next_token(parser->tokenizer);
                if(definition.type == TKN_LABEL_REF || definition.type == TKN_ADDR_LABEL_REF){
                    const Token label = definition;
                    if(label.type == TKN_ADDR_LABEL_REF) definition.type = TKN_LABEL_REF;
                    definition = resolve_token(parser->labels, definition);
                    if(definition.type == TKN_ERROR){
                        REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n", label.size, label.value.as_str);
                        return 1;
                    }
                    else if(definition.type != TKN_ULIT && definition.type != TKN_ILIT){
                        REPORT_ERROR(parser, "\n\t%.*s Expects Valid Number After =\n", macro.size, macro.value.as_str);
                        return 1;
                    }
                    if(label.type == TKN_ADDR_LABEL_REF){
                        definition.value.as_uint -= parser->program->size / 4;
                    }
                }
                const Operand op = parse_op_literal(definition);
                if(op.type != TKN_ULIT && op.type != TKN_ILIT){
                    REPORT_ERROR(parser, "\n\t%.*s Macro Rvalue Should Be Integer\n", macro.size, macro.value.as_str);
                    return 1;
                }
                definition.type = op.type;
                definition.value.as_uint = op.value.as_uint64;
                definition.size = 0;
                next = get_next_token(parser->tokenizer);
            }
            if(next.type == TKN_SPECIAL_SYM && next.value.as_char == ','){
                next = get_next_token(parser->tokenizer);
            }

            if(add_label(parser->labels, token, definition)){
                REPORT_ERROR(
                    parser, "\n\t%.*s Could Not Add Label '%.*s', It's Invalid Or It Already Exists\n",
                    macro.size, macro.value.as_str,
                    token.size, token.value.as_str
                );
                return 1;
            }
            definition.value.as_int += 1;
            token = next;
        }
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%endif"))){
        if(parser->macro_if_depth == 0){
            REPORT_ERROR(parser, "\n\tNo Macro If Statement Matches To Match This %s\n\n", "%endif");
            return 1;
        }
        parser->macro_if_depth -= 1;
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%start"))){
        parser->entry_point = parser->program->size / 4;
        return 0;
    }

    REPORT_ERROR(parser, "\n\tInvalid Macro Instruction '%.*s'\n\n", macro.size, macro.value.as_str);
    return 1;
}

int pre_parse_inst_operand(const Parser* parser, Token* _token, uint64_t absolute_program_position){
    Token token = *_token;
    if(token.type == TKN_LABEL_REF){
        if(token.size > 2){
            if(token.value.as_str[2] == '.'){
                REPORT_ERROR(parser, "\n\tCan Not Reference Local Label With '$', Only Relative Address References Are Allowed '%c'\n", '@');
                return 1;
            }
        }
        token = resolve_token(parser->labels, token);
        if(token.type == TKN_ERROR){
            REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", _token->size, _token->value.as_str);
            return 1;
        }
        if(token.type == TKN_STATIC_SIZE){
            token.value.as_uint = parser->static_memory->size;
            token.type = TKN_ULIT;
        }
    }
    else if(token.type == TKN_ADDR_LABEL_REF){
        token.type = TKN_LABEL_REF;
        token = resolve_token(parser->labels, token);
        if(token.type == TKN_ERROR){
            if(token.size > 1 && parser->local_labels){
                if(token.value.as_str[1] == '.'){
                    union {int16_t as_int16; uint16_t as_uint16; } stride;
                    token.type = TKN_RAW;
                    if(add_local_labelref(parser->local_labels, &stride.as_int16, token, absolute_program_position)){
                        REPORT_ERROR(parser, "\n\tCould Not Add Local Label Reference '%.*s'\n", token.size, token.value.as_str);
                        return 1;
                    }
                    *_token = (Token){.value.as_uint = stride.as_uint16, .type = TKN_ILIT, .size = 0};
                    return 0;
                }
            }
            REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", _token->size, _token->value.as_str);
            return 1;
        }
        if(token.type != TKN_INST_POSITION){
            REPORT_ERROR(parser, "\n\tInvalid Value For Relative Referencing %c", '\'');
            fprint_token(stderr, token);
            fprintf(stderr, "\'\n\n");
            return 1;
        }
        const int64_t v = token.value.as_uint - (parser->program->size / 4);
        if(v != (int16_t) v){
            REPORT_ERROR(parser, "\n\tLiteral Has To Be Up To 16 Bits Long, %"PRIi64" != %"PRIi16"\n\n", v, (int16_t) v);
            return 1;
        }
        const Register dummy = (Register){.as_uint64 = (int16_t)(v) & 0xFFFF};
        token.value.as_uint = dummy.as_uint64;
    }

    *_token = token;
    return 0;
}

// \returns the parsed instruction on success or INST_ERROR on failure
Inst parse_inst(Parser* parser, InstProfile inst_profile, const StringView inst_sv){

    Inst inst = inst_profile.opcode;
    int op_pos_in_inst = 1;
    int op_token_pos = 1;

    for ( ; inst_profile.op_profile;
    inst_profile.op_profile = (inst_profile.op_profile >> 8) & ~0XFF00000000000000)
    {
        const Token tokenRW = get_next_token(parser->tokenizer);
        Token token = tokenRW;

        if(pre_parse_inst_operand(parser, &token, parser->program->size + op_pos_in_inst))
            return INST_ERROR;

        switch (inst_profile.op_profile & 0XFF)
        {
        case EXPECT_OP_REG:{
            const int reg = get_reg(token);
            if(reg < 0){
                REPORT_ERROR(
                    parser,
                    "\n\tArgument %i Of Instruction %.*s Should Be Register, Got '%.*s' Of Type %s Instead\n\n",
                    op_token_pos, inst_sv.size, inst_sv.str, tokenRW.size, tokenRW.value.as_str, get_token_type_str(token.type)
                );
                return INST_ERROR;
            }
            inst |= (reg << (op_pos_in_inst * 8));
            op_pos_in_inst += 1;
            op_token_pos += 1;
        }   break;
        case EXPECT_OP_LIT:{
            if(token.type == TKN_STR){
                if(!parser->static_memory){
                    REPORT_ERROR(parser, "\n\tNo Static Memory Available%c\n", '\n');
                    return INST_ERROR;
                }
                if(token.size < 2 || token.value.as_str[token.size - 1] != '\"'){
                    REPORT_ERROR(parser, "\n\tMissing Closing %c\n\n", '\"');
                    return INST_ERROR;
                }
                const uint64_t spos = parser->static_memory->size;
                push_to_static(parser->static_memory, token);
                token.value.as_uint = spos;
                token.type = TKN_ULIT;
            }
            const Operand operand = parse_op_literal(token);
            if(operand.type == TKN_ERROR){
                REPORT_ERROR(
                    parser,
                    "\n\tArgument %i Of Instruction %.*s Should Be Literal, Got '%.*s' Of Type %s Instead\n\n",
                    op_token_pos, inst_sv.size, inst_sv.str, tokenRW.size, tokenRW.value.as_str, get_token_type_str(token.type)
                );
                return INST_ERROR;
            }
            if(operand.value.as_uint64 != operand.value.as_uint16){
                REPORT_ERROR(
                    parser,
                    "\n\tLiteral Has To Be Up To 16 Bits Long %"PRIu64" != %"PRIu16"\n\n",
                    operand.value.as_uint64, operand.value.as_uint16
                );
                return INST_ERROR;
            }
            inst |= operand.value.as_uint16 << (8 * op_pos_in_inst);
            op_pos_in_inst += 2;
            op_token_pos += 1;
        }   break;
        
        case EXPECT_OP_EITHER:{
            if(token.type == TKN_STR){
                if(token.size < 2 || token.value.as_str[token.size - 1] != '\"'){
                    REPORT_ERROR(parser, "\n\tMissing Closing %c\n\n", '\"');
                    return INST_ERROR;
                }
                if(!parser->static_memory){
                    REPORT_ERROR(parser, "\n\tNo Static Memory Available%c\n", '\n');
                    return INST_ERROR;
                }
                const uint64_t op_value = parser->static_memory->size;
                push_to_static(parser->static_memory, token);
                inst |= (op_value & 0XFFFF) << 8;
                inst |= HINT_LIT << 31;
                op_token_pos += 1;
                op_pos_in_inst += 3;
                break;
            }
            const int reg = get_reg(token);
            if(reg < 0){
                const Operand operand = parse_op_literal(token);
                if(operand.type == TKN_ERROR){
                    REPORT_ERROR(
                        parser,
                        "\n\tArgument %i Of Instruction %.*s Should Be Register Or Literal, Got '%.*s' Of Type %s Instead\n\n",
                        op_token_pos, inst_sv.size, inst_sv.str, tokenRW.size, tokenRW.value.as_str, get_token_type_str(token.type)
                    );
                    return INST_ERROR;
                }
                if(operand.value.as_uint16 != operand.value.as_uint64){
                    REPORT_ERROR(
                        parser,
                        "\n\tLiteral Has To Be Up To 16 Bits Long %"PRIu64" != %"PRIu16"\n\n",
                        operand.value.as_uint64, operand.value.as_uint16
                    );
                    return INST_ERROR;
                }
                inst |= operand.value.as_uint16 << (8 * op_pos_in_inst);
                inst |= HINT_LIT << 31;
                op_pos_in_inst += 3;
                op_token_pos += 1;
                break;
            }
            inst |= (reg << (op_pos_in_inst * 8));
            inst |= HINT_REG << 31;
            op_pos_in_inst += 3;
            op_token_pos += 1;
        }   break;
        default:
            break;
        }
    }

    if(op_pos_in_inst > 5){
        REPORT_ERROR(parser, "INTERNAL ERROR OCURRED\n\tIf You're An User Beware That This Is Not Your Fault, "
        "Instead It's A Problem With The Compiler Implementation\n%c", '\n');
        fprintf(stderr, "[INTERNAL ERROR] "__FILE__":%i:10: Instruction Size Overflow, Instance Of '%.*s' Has Size Of %i Bytes\n",
        __LINE__, inst_sv.size, inst_sv.str, (op_pos_in_inst - 1));
        return INST_ERROR;
    }

    return inst;
}

// \return 1 on error or 0 otherwise
int parse_file(Parser* parser, Mc_stream_t* files_stream){

    VIRTUAL_DEBUG_LOG("parsing file '%s'\n", (char*) parser->file_path);

    InstProfile inst_profile = (InstProfile){.opcode = INST_ERROR, .op_profile = OP_PROFILE_NONE};

    Token token = get_next_token(parser->tokenizer);
    for(;
        (token.type != TKN_NONE) && (token.type != TKN_ERROR);
        token = get_next_token(parser->tokenizer)){

        if(token.type == TKN_EMPTY){
            continue;
        }
        if(token.type == TKN_LABEL_REF){
            const Token tmp = resolve_token(parser->labels, token);
            if(tmp.type == TKN_ERROR){
                REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", token.size, token.value.as_str);
                return 1;
            }
            token = tmp;
        }
        if(token.type == TKN_MACRO_INST){
            StringView next_path_sv = (StringView){.str = NULL};
            if(parse_macro(parser, token, &next_path_sv)){
                return 1;
            }
            if(next_path_sv.str != NULL){
                StringView mother_directory = (StringView){.str = parser->file_path, .size = parser->file_path_size};
                for(unsigned int i = 0; i < mother_directory.size; i+=1){
                    const char c = mother_directory.str[mother_directory.size - i - 1];
                    if(c == '/'){
                        mother_directory.size -= i;
                        break;
                    }
                }
                const uint64_t previous_file_stream_size = files_stream->size;
                const size_t file_pos = (size_t)((uint8_t*)(parser->file_path) - (uint8_t*)(files_stream->data));
                const char* const new_file = read_file_relative(files_stream, mother_directory, next_path_sv);
                if(new_file == NULL){
                    parser->file_path = (char*) mc_stream_on(files_stream, file_pos);
                    REPORT_ERROR(
                        parser, "Could Not Read File '%.*s%.*s', Invalid File Or File Path\n\n",
                        mother_directory.size, mother_directory.str, next_path_sv.size, next_path_sv.str
                    );
                    return 1;
                }
                const Tokenizer previous_tokenizer_state = *(parser->tokenizer);
                const int macro_if_depth = parser->macro_if_depth;
                const int previous_file_path_size = parser->file_path_size;
		        parser->file_path_size = mother_directory.size + next_path_sv.size;
                parser->macro_if_depth = 0;
                *(parser->tokenizer) = (Tokenizer){
                    .data = (char*)((uint8_t*)(new_file) + mother_directory.size + next_path_sv.size + 1 + sizeof(uint32_t)),
                    .pos = 0, .line = 0, .column = 0
                };
                parser->file_path = (char*)((uint8_t*)(new_file) + sizeof(uint32_t));
                if(parse_file(parser, files_stream))
                    return 1;
                files_stream->size = previous_file_stream_size;
                parser->file_path = (char*)((uint8_t*)(files_stream->data) + file_pos);
                parser->file_path_size = previous_file_path_size;
                parser->macro_if_depth = macro_if_depth;
                *(parser->tokenizer) = previous_tokenizer_state;
                parser->tokenizer->data = (char*) mc_stream_on(files_stream, file_pos + parser->file_path_size + 1);
            }
            continue;
        }
        if(token.type == TKN_RAW && token.size > 1){
            if(token.value.as_str[0] == '.'){
                if(!parser->local_labels){
                    REPORT_ERROR(parser, "\n\tNo Local Label Support%c\n", '\n');
                    return 1;
                }
                const Token next_token = get_next_token(parser->tokenizer);
                if(next_token.type != TKN_SPECIAL_SYM || next_token.value.as_char != ':'){
                    REPORT_ERROR(parser, "\n\tExpected '%c' After Label Identifier\n", ':');
                    return 1;
                }
                if(solve_local_label(parser->local_labels, parser->program->data, token, parser->program->size / 4)){
                    REPORT_ERROR(parser, "\n\tCould Not Add Local Label '%.*s'\n", token.size, token.value.as_str);
                    return 1;
                }
                continue;
            }
        }

        inst_profile = get_inst_profile(token);
        if(inst_profile.opcode == INST_ERROR){
            const Token next_token = get_next_token(parser->tokenizer);
            if((next_token.type == TKN_SPECIAL_SYM) && (token.type == TKN_RAW)){
                if(next_token.value.as_char == ':'){
                    if(add_label(parser->labels, token, (Token){.value.as_uint = parser->program->size / 4, .type = TKN_INST_POSITION}))
                    {
                        REPORT_ERROR(
                            parser,
                            "\n\tCould Not Add Label '%.*s', Label Is Either Invalid Or Has Already Been Defined\n",
                            token.size, token.value.as_str
                        );
                        return 1;
                    }
                    if(parser->local_labels){
                        const Label* unsolved_local_label = get_missing_local_label(parser->local_labels);
                        if(unsolved_local_label){
                            const Label l = get_label_from_raw_data(parser->local_labels->data);
                            const char* const unsolved_local_label_name = (char*) (((uintptr_t) unsolved_local_label) + l.str);
                            REPORT_ERROR(parser, "\n\tUnsolved Local Label '%.*s'\n", l.size, unsolved_local_label_name);
                            return 1;
                        }
                        parser->local_labels->size = 0;
                    }
                    continue;
                }
            }
            REPORT_ERROR(parser, "\n\tExpected Instruction, Got '%.*s' Instead\n\n", token.size, token.value.as_str);
            return 1;
        }
        
        const Inst inst = parse_inst(parser, inst_profile, (StringView){.str = token.value.as_str, .size = token.size});

        if(inst == INST_ERROR){
            return 1;
        }
        mc_stream(parser->program, &inst, sizeof(inst));
    }

    if(token.type == TKN_ERROR){
        REPORT_ERROR(parser, "Invalid Token%c\n", ' ');
        return 1;
    }

    if(parser->local_labels){
        const Label* unsolved_local_label = get_missing_local_label(parser->local_labels);
        if(unsolved_local_label){
            const Label l = get_label_from_raw_data(parser->local_labels->data);
            const char* const unsolved_local_label_name = (char*) (((uintptr_t) unsolved_local_label) + l.str);
            REPORT_ERROR(parser, "\n\tUnsolved Local Label '%.*s'\n", l.size, unsolved_local_label_name);
            return 1;
        }
        parser->local_labels->size = 0;
    }

    return 0;
}


#endif
