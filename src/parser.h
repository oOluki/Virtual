#ifndef VIRTUAL_PARSER_H
#define VIRTUAL_PARSER_H

#include "lexer.h"
#include <stdio.h>
#include "core.h"
#include "labels.h"

#define COMP_TKN(TKN1, TKN2) mc_compare_token(TKN1, TKN2, 0)

#define REPORT_ERROR(PARSER, STR, ...) do{\
    fprintf(\
        stderr,\
        "[ERROR] %s:%i:%i :  " STR,\
        PARSER->file_path, PARSER->tokenizer->line + 1,\
        PARSER->tokenizer->column + 1, __VA_ARGS__\
    );\
} while(0)

#define REP_INVALID_REGID(PARSER, TOKEN) REPORT_ERROR(PARSER, "\n\tInvalid Register Identifier '%.*s', No Such Register\n", TOKEN.size, TOKEN.value.as_str)

enum ParsingFlags{

    FLAG_NONE = 0,
    FLAG_TEST  = 1 << 0,
    FLAG_ADD_SYM_TABLE = 1 << 1
};

enum OpHint{

    HINT_NONE = 0,
    HINT_8BIT,
    HINT_16BIT,
    HINT_32BIT,

    HINT_STR,
    HINT_NUM

};

enum Expects{
    EXPECT_ANY = 0,
    EXPECT_INST,
    EXPECT_OP_REG,
    EXPECT_OP_LIT,
    EXPECT_OP_STR,
    EXPECT_IDENTIFIER,
};

typedef struct Operand{
    
    Register value;
    uint8_t type;

} Operand;

typedef struct InstProfile{

    uint8_t   opcode;
    OpProfile profile;

} InstProfile;

typedef struct Parser
{
    char* file_path;
    int file_path_size;
    Mc_stream_t* labels;
    Tokenizer* tokenizer;
    uint32_t flags;
    int macro_if_depth;
    uint64_t entry_point;
} Parser;


InstProfile parse_inst(const Token inst_token){
    if(inst_token.type != TKN_RAW)            return (InstProfile){INST_ERROR , 0};

    if(COMP_TKN(inst_token, MKTKN("NOP")))    return (InstProfile){INST_NOP   , OP_PROFILE_NONE};
    if(COMP_TKN(inst_token, MKTKN("HALT")))   return (InstProfile){INST_HALT  , OP_PROFILE_NONE};
    if(COMP_TKN(inst_token, MKTKN("MOV8")))   return (InstProfile){INST_MOV8  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MOV16")))  return (InstProfile){INST_MOV16 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MOV32")))  return (InstProfile){INST_MOV32 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MOV")))    return (InstProfile){INST_MOV   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MOVV")))   return (InstProfile){INST_MOVV  , OP_PROFILE_RL};
    if(COMP_TKN(inst_token, MKTKN("PUSH")))   return (InstProfile){INST_PUSH  , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("PUSHV")))  return (InstProfile){INST_PUSHV , OP_PROFILE_L};
    if(COMP_TKN(inst_token, MKTKN("POP")))    return (InstProfile){INST_POP   , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("GET")))    return (InstProfile){INST_GET   , OP_PROFILE_RL};
    if(COMP_TKN(inst_token, MKTKN("WRITE")))  return (InstProfile){INST_WRITE , OP_PROFILE_RL};
    if(COMP_TKN(inst_token, MKTKN("GSP")))    return (InstProfile){INST_GSP   , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("STATIC"))) return (InstProfile){INST_STATIC, OP_PROFILE_L};
    if(COMP_TKN(inst_token, MKTKN("READ8")))  return (InstProfile){INST_READ8 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("READ16"))) return (InstProfile){INST_READ16, OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("READ32"))) return (InstProfile){INST_READ32, OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("READ")))   return (InstProfile){INST_READ  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SET8")))   return (InstProfile){INST_SET8  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SET16")))  return (InstProfile){INST_SET16 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SET32")))  return (InstProfile){INST_SET32 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SET")))    return (InstProfile){INST_SET   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("TEST")))   return (InstProfile){INST_TEST  , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("NOT")))    return (InstProfile){INST_NOT   , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("NEG")))    return (InstProfile){INST_NEG   , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("AND")))    return (InstProfile){INST_AND   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("NAND")))   return (InstProfile){INST_NAND  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("OR")))     return (InstProfile){INST_OR    , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("XOR")))    return (InstProfile){INST_XOR   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("BSHIFT"))) return (InstProfile){INST_BSHIFT, OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("JMP")))    return (InstProfile){INST_JMP   , OP_PROFILE_L};
    if(COMP_TKN(inst_token, MKTKN("JMPF")))   return (InstProfile){INST_JMPIF , OP_PROFILE_RL};
    if(COMP_TKN(inst_token, MKTKN("JMPFN")))  return (InstProfile){INST_JMPIFN, OP_PROFILE_RL};
    if(COMP_TKN(inst_token, MKTKN("CALL")))   return (InstProfile){INST_CALL  , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("RET")))    return (InstProfile){INST_RET   , OP_PROFILE_NONE};
    if(COMP_TKN(inst_token, MKTKN("ADD8")))   return (InstProfile){INST_ADD8  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SUB8")))   return (InstProfile){INST_SUB8  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MUL8")))   return (InstProfile){INST_MUL8  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("ADD16")))  return (InstProfile){INST_ADD16 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SUB16")))  return (InstProfile){INST_SUB16 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MUL16")))  return (InstProfile){INST_MUL16 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("ADD32")))  return (InstProfile){INST_ADD32 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SUB32")))  return (InstProfile){INST_SUB32 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MUL32")))  return (InstProfile){INST_MUL32 , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("ADD")))    return (InstProfile){INST_ADD   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SUB")))    return (InstProfile){INST_SUB   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MUL")))    return (InstProfile){INST_MUL   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("DIVI")))   return (InstProfile){INST_DIVI  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("DIVU")))   return (InstProfile){INST_DIVU  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("ADDF")))   return (InstProfile){INST_ADDF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SUBF")))   return (InstProfile){INST_SUBF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("MULF")))   return (InstProfile){INST_MULF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("DIVF")))   return (InstProfile){INST_DIVF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("EQI")))    return (InstProfile){INST_EQI   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("EQU")))    return (InstProfile){INST_EQU   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("EQF")))    return (InstProfile){INST_EQF   , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("BIGI")))   return (InstProfile){INST_BIGI  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("BIGU")))   return (InstProfile){INST_BIGU  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("BIGF")))   return (InstProfile){INST_BIGF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SMLI")))   return (InstProfile){INST_SMLI  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SMLU")))   return (InstProfile){INST_SMLU  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("SMLF")))   return (InstProfile){INST_SMLF  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("CASTIU"))) return (InstProfile){INST_CASTIU, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CASTIF"))) return (InstProfile){INST_CASTIF, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CASTUI"))) return (InstProfile){INST_CASTUI, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CASTUF"))) return (InstProfile){INST_CASTUF, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CASTFI"))) return (InstProfile){INST_CASTFI, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CASTFU"))) return (InstProfile){INST_CASTFU, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CF3264"))) return (InstProfile){INST_CF3264, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("CF6432"))) return (InstProfile){INST_CF6432, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("MEMSET"))) return (InstProfile){INST_MEMSET, OP_PROFILE_RRR};
    if(COMP_TKN(inst_token, MKTKN("MEMCPY"))) return (InstProfile){INST_MEMCPY, OP_PROFILE_RRR};
    if(COMP_TKN(inst_token, MKTKN("MEMMOV"))) return (InstProfile){INST_MEMMOV, OP_PROFILE_RRR};
    if(COMP_TKN(inst_token, MKTKN("MEMCMP"))) return (InstProfile){INST_MEMCMP, OP_PROFILE_RRR};
    if(COMP_TKN(inst_token, MKTKN("MALLOC"))) return (InstProfile){INST_MALLOC, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("FREE")))   return (InstProfile){INST_FREE  , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("FOPEN")))  return (InstProfile){INST_FOPEN , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("FCLOSE"))) return (InstProfile){INST_FCLOSE, OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("PUTC")))   return (InstProfile){INST_PUTC  , OP_PROFILE_RR};
    if(COMP_TKN(inst_token, MKTKN("GETC")))   return (InstProfile){INST_GETC  , OP_PROFILE_R};

    if(COMP_TKN(inst_token, MKTKN("SYS")))    return (InstProfile){INST_SYS   , OP_PROFILE_L};
    if(COMP_TKN(inst_token, MKTKN("DISREG"))) return (InstProfile){INST_DISREG, OP_PROFILE_R};

    return (InstProfile){INST_ERROR, 0};
}

uint64_t parse_hexadecimal(Parser* parser, const Token token){

    uint64_t n = 0;
    
    for(size_t i = 0; i < token.size; i+=1){
        const uint8_t digit = get_hex_digit(token.value.as_str[i]);

        if(digit == 255){
            parser->flags |= FLAG_TEST;
            return 0;
        }

        n = (n << 4) | digit;
    }

    return n;
}

static inline int get_major_reg_identifier(const Token token){
    if(mc_compare_token(token, MKTKN("RA"), 1))  return RA;
    if(mc_compare_token(token, MKTKN("RB"), 1))  return RB;
    if(mc_compare_token(token, MKTKN("RC"), 1))  return RC;
    if(mc_compare_token(token, MKTKN("RD"), 1))  return RD;
    if(mc_compare_token(token, MKTKN("RE"), 1))  return RE;
    if(mc_compare_token(token, MKTKN("RF"), 1))  return RF;
    if(mc_compare_token(token, MKTKN("RSP"), 1)) return RSP;
    if(mc_compare_token(token, MKTKN("RIP"), 1)) return RIP;
    return -1;
}

int get_reg(const Token token){
    if(token.type == TKN_REG) return (int)token.value.as_uint;
    if(token.type != TKN_RAW) return -1;
    const int reg = get_major_reg_identifier(token);
    if(reg >= 0){
        if(token.size == 2){
            return reg;
        }
        if(token.size == 3){
            const int i = get_digit(token.value.as_str[2]);
            if((i < 0) || (i > 8)){
                return -1;
            }
            return reg + i;
        }
    }
    return -1;
}

uint64_t get_expectations(int inst_profile){
    switch (inst_profile)
    {
    case OP_PROFILE_R:    return EXPECT_OP_REG;
    case OP_PROFILE_RR :  return EXPECT_OP_REG | ((EXPECT_OP_REG << 8) & ~0XFF);
    case OP_PROFILE_RRR:  return EXPECT_OP_REG | ((EXPECT_OP_REG << 8) & ~0XFF) | ((EXPECT_OP_REG << 16) & ~0XFFFF);
    case OP_PROFILE_RL:   return EXPECT_OP_REG | ((EXPECT_OP_LIT << 8) & ~0XFF);
    case OP_PROFILE_L:    return EXPECT_OP_LIT;
    default:              return EXPECT_ANY;
    }
    return EXPECT_ANY;
}

Operand parse_op_literal(Parser* parser, Token token, int hint){
    
    if((token.type == TKN_FLIT) || (token.type == TKN_ILIT) || (token.type == TKN_ULIT)){
        return (Operand){.value.as_uint64 = token.value.as_uint, .type = token.type};
    }
    if(token.type != TKN_RAW){
        return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
    }

    if(token.value.as_str[0] == '0'){
        if(token.value.as_str[1] == 'x' || token.value.as_str[1] == 'X'){
            const uint64_t hex = parse_hexadecimal(
                parser,
                (Token){
                    .value.as_str = token.value.as_str + 2,
                    .size = token.size - 2
                }
            );
            if(parser->flags & FLAG_TEST){
                REPORT_ERROR(parser, "Invalid Token '%.*s'\n", token.size, token.value.as_str);
                parser->flags &= ~FLAG_TEST;
                return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
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
        switch (hint)
        {
        case HINT_8BIT:
            return is_negative?
                (Operand){.value.as_int8  = (int8_t)(-first_part), .type = TKN_ILIT}:
                (Operand){.value.as_uint8 = (int8_t)( first_part), .type = TKN_ULIT};
        case HINT_16BIT:
            return is_negative?
                (Operand){.value.as_int8  = (int16_t)(-first_part), .type = TKN_ILIT}:
                (Operand){.value.as_uint8 = (int16_t)( first_part), .type = TKN_ULIT};
        case HINT_32BIT:
            return is_negative?
                (Operand){.value.as_int8  = (int32_t)(-first_part), .type = TKN_ILIT}:
                (Operand){.value.as_uint8 = (int32_t)( first_part), .type = TKN_ULIT};
        default:
            return is_negative?
                (Operand){.value.as_int64  = (int64_t)(-first_part), .type = TKN_ILIT}:
                (Operand){.value.as_uint64 =            first_part , .type = TKN_ULIT};
        }
        
    }
    if(hint == HINT_32BIT){
       const float output = (float)(second_part) + (float)(first_part) / (float)(_10n1);
    
        return (Operand){.value.as_float32 = is_negative? -output : output, .type = TKN_FLIT}; 
    }

    const double output = (double)(second_part) + (double)(first_part) / (double)(_10n1);
    
    return (Operand){.value.as_float64 = is_negative? -output : output, .type = TKN_FLIT};
}

int parse_macro(Parser* parser, const uint64_t program_position, const Token macro, StringView* include_path){
    
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
        const Token arg2 = get_next_token(parser->tokenizer);
        if(arg1.type != TKN_RAW){
            REPORT_ERROR(parser, "\n\tLabel Identifier Is Either Missing Or Invalid%c\n\n", ' ');
            return 1;
        }
        else if(arg2.type == TKN_NONE){
            REPORT_ERROR(parser, "\n\tMissing Definition For '%.*s' Label\n\n", arg1.size, arg1.value.as_str);
            return 1;
        }
        if(add_label(parser->labels, arg1, arg2, HINT_NONE)){
            REPORT_ERROR(
                parser, "\n\tInvalid Label Or Definition '%s %.*s %.*s'\n\tNOT: You Can Not Redifined Already Labeled Labels\n\n",
                "%label",
                arg1.size, arg1.value.as_str, arg2.size, arg2.value.as_str
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
        if(add_label(parser->labels, arg1, (Token){.type = TKN_EMPTY}, HINT_NONE)){
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
    if(COMP_TKN(macro, MKTKN("%endif"))){
        if(parser->macro_if_depth == 0){
            REPORT_ERROR(parser, "\n\tNo Macro If Statement Matches To Match This %s\n\n", "%endif");
            return 1;
        }
        parser->macro_if_depth -= 1;
        return 0;
    }
    if(COMP_TKN(macro, MKTKN("%start"))){
        parser->entry_point = program_position;
        return 0;
    }

    REPORT_ERROR(parser, "\n\tInvalid Macro Instruction '%.*s'\n\n", macro.size, macro.value.as_str);
    return 1;
}

// \return 1 on error or 0 otherwise
int parse_file(Parser* parser, Mc_stream_t* program, Mc_stream_t* static_memory, Mc_stream_t* files_stream){

    Operand operand;
    int opv = 0;
    InstProfile inst = (InstProfile){.opcode = INST_ERROR, .profile = OP_PROFILE_NONE};
    Token inst_token = (Token){.value.as_str = NULL};
    uint64_t expects = EXPECT_ANY;

    Token token = get_next_token(parser->tokenizer);
    for(;
        (token.type != TKN_NONE) && (token.type != TKN_ERROR);
        token = get_next_token(parser->tokenizer)){

        while (token.type == TKN_EMPTY) continue;


        if(token.type == TKN_LABEL_REF){
            const Token tmp = resolve_token(parser->labels, token);
            if(tmp.type == TKN_ERROR){
                REPORT_ERROR(parser, "\n\tCould Not Resolve Label '%.*s'\n\n", token.size, token.value.as_str);
                return 1;
            }
            token = tmp;
        }

        switch (expects & 0XFF)
        {
        default:
            if(token.type == TKN_MACRO_INST){
                StringView next_path_sv = (StringView){.str = NULL};
                if(parse_macro(parser, program->size, token, &next_path_sv)){
                    return 1;
                }
                if(next_path_sv.str != NULL){
                    StringView mother_directory = (StringView){.str = parser->file_path, .size = parser->file_path_size};
                    for(int i = 0; i < mother_directory.size; i+=1){
                        const char c = mother_directory.str[mother_directory.size - i - 1];
                        if(c == '/'){
                            mother_directory.size -= i;
                            break;
                        }
                    }
                    const uint64_t previous_file_stream_size = files_stream->size;
                    const char* const new_file = read_file_relative(files_stream, mother_directory, next_path_sv);
                    if(new_file == NULL){
                        REPORT_ERROR(
                            parser, "Could Not Read File '%.*s%.*s', Invalid File Or File Path\n\n",
                            mother_directory.size, mother_directory.str, next_path_sv.size, next_path_sv.str
                        );
                        return 1;
                    }
                    const Tokenizer previous_tokenizer_state = *(parser->tokenizer);
                    const int macro_if_depth = parser->macro_if_depth;
                    const size_t file_pos = (size_t)((uint8_t*)(parser->file_path) - (uint8_t*)(files_stream->data));
                    const int previous_file_path_size = parser->file_path_size;
                    parser->macro_if_depth = 0;
                    *(parser->tokenizer) = (Tokenizer){
                        .data = (char*)((uint8_t*)(new_file) + mother_directory.size + next_path_sv.size + 1 + sizeof(uint32_t)),
                        .pos = 0, .line = 0, .column = 0
                    };
                    parser->file_path = (char*)((uint8_t*)(new_file) + sizeof(uint32_t));
                    if(parse_file(parser, program, static_memory, files_stream))
                        return 1;
                    files_stream->size = previous_file_stream_size;
                    parser->file_path = (char*)((uint8_t*)(files_stream->data) + file_pos);
                    parser->file_path_size = previous_file_path_size;
                    parser->macro_if_depth = macro_if_depth;
                    *(parser->tokenizer) = previous_tokenizer_state;
                    parser->tokenizer->data = (char*)((uint8_t*)(files_stream->data) + file_pos + parser->file_path_size + 1);
                }
                continue;
            }
        case EXPECT_INST:
            inst = parse_inst(token);
            if(inst.opcode == INST_ERROR){
                const Token next_token = get_next_token(parser->tokenizer);
                if((next_token.type == TKN_SPECIAL_SYM) && (token.type == TKN_RAW)){
                    if(COMP_TKN(next_token, MKTKN(":"))){
                        if(
                            add_label(
                                parser->labels, token,
                                (Token){.value.as_uint = program->size,
                                .type = TKN_ULIT},
                                0
                            )
                        ) {
                            REPORT_ERROR(
                                parser,
                                "\n\tCould Not Add Label '%.*s', Label Is Either Invalid Or Has Already Been Defined\n",
                                token.size, token.value.as_str
                            );
                            return 1;
                        }
                        break;
                    }
                }
                REPORT_ERROR(parser, "\n\tExpected Instruction, Got '%.*s' Instead\n\n", token.size, token.value.as_str);
                return 1;
            }
            opv = 0;
            inst_token = token;
            mc_stream(program, &inst.opcode, 1);
            expects = get_expectations(inst.profile);
            break;
        case EXPECT_OP_REG:
            operand.value.as_int64 = get_reg(token);
            if(operand.value.as_int64 < 0){
                REPORT_ERROR(
                    parser,
                    "\n\tArgument %i Of Instruction %.*s Should Be Register, Got '%.*s' Instead\n\n",
                    opv + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
                );
                return 1;
            }
            mc_stream(program, &(operand.value.as_uint8), 1);
            opv += 1;
            expects = (expects >> 8) & ~0XFF00000000000000;
            break;
        case EXPECT_OP_LIT:
            if(token.type == TKN_STR){
                if(inst.opcode != INST_STATIC){
                    REPORT_ERROR(parser, "\n\tInstruction %.*s Can't Take String As Argument\n\n", inst_token.size, inst_token.value.as_str);
                    return 1;
                }
                if(token.value.as_str[token.size - 1] != '\"'){
                    REPORT_ERROR(parser, "\n\tMissing Closing %c\n\n", '\"');
                    return 1;
                }
                operand.value.as_uint64 = static_memory->size;
                mc_stream(static_memory, token.value.as_str + 1, token.size - 2);
                mc_stream_str(static_memory, "");
                mc_stream(program, &(operand.value.as_uint64), 8);
                expects = (expects >> 8) & ~0XFF00000000000000;
                break;
            }
            operand = parse_op_literal(
                parser,
                token,
                (inst.opcode == INST_MOV8)  * HINT_8BIT  +
                (inst.opcode == INST_MOV16) * HINT_16BIT +
                (inst.opcode == INST_MOV32) * HINT_32BIT
            );
            if(operand.type == TKN_ERROR){
                REPORT_ERROR(
                    parser,
                    "\n\tArgument %i Of Instruction %.*s Should Be A Literal, Got '%.*s' Instead\n\n",
                    opv + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
                );
                return 1;
            }
            mc_stream(program, &(operand.value.as_uint16), 2);
            opv += 1;
            expects = (expects >> 8) & ~0XFF00000000000000;
            break;
        }



    }

    return token.type == TKN_ERROR;
}


#endif
