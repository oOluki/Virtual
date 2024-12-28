#ifndef VIRTUAL_PARSER_H
#define VIRTUAL_PARSER_H

#include "lexer.h"
#include <stdio.h>
#include "core.h"
#include "labels.h"

#define COMP_TKN(TKN1, TKN2) mc_compare_token(TKN1, TKN2, 0)

#define REPORT_ERROR(PARSER, STR, ...)   fprintf(stderr, "[ERROR] In File '%s', On Line %i:\n\t" STR, PARSER->file_path, PARSER->tokenizer->line + 1, __VA_ARGS__)

#define REPORT_WARNING(PARSER, STR, ...) fprintf(stderr, "[WARNING] In File '%s', On Line %i:\n\t" STR, PARSER->file_path, PARSER->tokenizer->line + 1, __VA_ARGS__)

#define REP_INVALID_REGID(PARSER, TOKEN) REPORT_ERROR(PARSER, "[ERROR] Invalid Register Identifier '%.*s', No Such Register\n", TOKEN.size, TOKEN.value.as_str)



enum ArgFlags{

    FLAG_NONE = 0,

    FLAG_EXPECT_INST = 1 << 0,
    FLAG_EXPECT_REG  = 1 << 1,
    FLAG_EXPECT_LIT  = 1 << 2,

    FLAG_ERROR = 1 << 3,
    FLAG_TEST  = 1 << 4,

    FLAG_ADD_SYM_TABLE = 1 << 5
};

enum OpHint{

    HINT_NONE = 0,
    HINT_8BIT,
    HINT_16BIT,
    HINT_32BIT,

    HINT_STR,
    HINT_NUM

};

typedef struct Operand{
    
    Register value;
    uint8_t type;

} Operand;

typedef struct InstProfile{

    uint8_t   inst;
    OpProfile profile;

} InstProfile;

typedef struct Parser
{
    const char* file_path;
    Mc_stream_t* labels;
    Tokenizer* tokenizer;
    int flags;
    uint64_t entry_point;
    InstProfile current_inst;
    int num_of_expected_operands;
} Parser;


InstProfile get_inst_type(const Token inst_token){

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
    if(COMP_TKN(inst_token, MKTKN("CALL")))   return (InstProfile){INST_JMP   , OP_PROFILE_R};
    if(COMP_TKN(inst_token, MKTKN("RET")))    return (InstProfile){INST_JMPIF , OP_PROFILE_NONE};
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

uint64_t parse_uint(Parser* parser, const Token token){

    uint64_t _10n = 1;
    uint64_t output = 0;
    
    for(int i = token.size - 1; i > -1; i-=1){

        const int digit = get_digit(token.value.as_str[i]);

        if(digit < 0){
            parser->flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    return output;
}

double parse_float(Parser* parser, Token token){
    int dot_pos = -1;
    for(int i = 0; i < token.size; i+=1){
        if(token.value.as_str[i] == '.'){
            if(dot_pos != -1){
                parser->flags |= FLAG_TEST;
                return 0;
            }
            dot_pos = i;
            break;
        }
    }

    if(token.value.as_str[token.size - 1] == 'f') token.size -= 1;

    uint64_t _10n = 1;
    double output = 0;
    
    for(int i = token.size - 1; i > dot_pos; i-=1){

        const int digit = get_digit(token.value.as_str[i]);

        if(digit < 0){
            parser->flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    if(dot_pos < 0) return output;

    output /= _10n;
    _10n = 1;

    for(int i = dot_pos - 1; i > -1; i-=1){
        const int digit = get_digit(token.value.as_str[i]);

        if(digit < 0){
            parser->flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    return output;
}

int get_major_reg_identifier(const Token token){
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

Operand parse_operand(Parser* parser, Token token, int hint){

    if((token.type == TKN_ERROR) || (token.type == TKN_NONE))
        return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
    
    if(token.type == TKN_LABEL_REF){
        token = resolve_token(parser->labels, token);
        if(token.type == TKN_ERROR){
            REPORT_ERROR(parser, "Could Not Resolve Label '%.*s'\n", token.size, token.value.as_str);
            return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
        }
        return (Operand){.value.as_uint64 = token.value.as_uint, .type = token.type};
    }

    if(token.value.as_str[0] == '0'){
        if(token.value.as_str[1] == 'x' || token.value.as_str[1] == 'X'){
            const uint64_t hex = parse_hexadecimal(parser, token);
            if(parser->flags & FLAG_TEST){
                REPORT_ERROR(parser, "Invalid Token '%.*s'\n", token.size, token.value.as_str);
                parser->flags &= ~FLAG_TEST;
                return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
            }
            return (Operand){.value.as_uint64 = hex, .type = TKN_ULIT};
        }
    }

    const int reg = get_major_reg_identifier(token);
    if(reg >= 0){
        if(token.size == 2){
            return (Operand){.value.as_uint8 = reg, .type = TKN_REG};
        }
        if(token.size == 3){
            const int i = get_digit(token.value.as_str[2]);
            if((i < 0) || (i > 8)){
                return (Operand){.value.as_uint8 = 0, .type = TKN_ERROR};
            }
            return (Operand){.value.as_uint8 = reg + i, .type = TKN_REG};
        }
        return (Operand){.value.as_uint8 = 0, .type = TKN_ERROR};
    }


    int is_float = 0;

    for(size_t i = 0; (i < token.size) && !is_float; i+=1){
        is_float = (token.value.as_str[i] == '.');
    }

    Register v;
    int type = TKN_ERROR;

    if(is_float){
        if(hint == HINT_NONE)       v.as_float64 = (double)(parse_float(parser, token));
        else if(hint == HINT_32BIT) v.as_float32 = (float )(parse_float(parser, token));
        else                        return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
        type = TKN_FLIT;
    }
    else if(token.value.as_str[0] == '-'){
        switch (hint)
        {
        case HINT_8BIT : v.as_int8  = -((int8_t )parse_uint(parser, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1})); break;
        case HINT_16BIT: v.as_int16 = -((int16_t)parse_uint(parser, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1})); break;
        case HINT_32BIT: v.as_int32 = -((int32_t)parse_uint(parser, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1})); break;
        default:         v.as_int64 = -((int64_t)parse_uint(parser, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1})); break;
        }
        type = TKN_ILIT;
    }
    else{
        switch (hint)
        {
        case HINT_8BIT : v.as_uint8  = (uint8_t )(parse_uint(parser, token)); break;
        case HINT_16BIT: v.as_uint16 = (uint16_t)(parse_uint(parser, token)); break;
        case HINT_32BIT: v.as_uint32 = (uint32_t)(parse_uint(parser, token)); break;
        default:         v.as_uint64 = (uint64_t)(parse_uint(parser, token)); break;
        }
        type = TKN_ULIT;
    }

    

    if(!(parser->flags & FLAG_TEST)) 
        return (Operand){.value = v, .type = type};

    parser->flags &= ~FLAG_TEST;
    
    return (Operand){.value.as_uint64 = 0, .type = TKN_ERROR};
}

// \return 0 on success or 1 on error
int parse_instruction(Parser* parser, const Token inst_token, Mc_stream_t* streambuff, Mc_stream_t* static_memory){

    const InstProfile inst = parser->current_inst;

    const int hint =
        (inst.inst == INST_MOV8 ) * HINT_8BIT  +
        (inst.inst == INST_MOV16) * HINT_16BIT +
        (inst.inst == INST_MOV32) * HINT_32BIT;

    Register op1;
    Register op2;
    Register op3;
    Operand op;

    int op_count = 0;

    Token token;


    switch (inst.profile)
    {
    case OP_PROFILE_RRR:
        token = get_next_token(parser->tokenizer);
        op = parse_operand(parser, token, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                parser,
                "Argument %i Of Instruction '%.*s' Should Be A Register, Got '%.*s' Instead\n\n",
                op_count + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
            );
            return 1;
        }
        op3.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_RR:
        token = get_next_token(parser->tokenizer);
        op = parse_operand(parser, token, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                parser,
                "Argument %i Of Instruction '%.*s' Should Be A Register, Got '%.*s' Instead\n\n",
                op_count + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
            );
            return 1;
        }
        op2.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_R:
        token = get_next_token(parser->tokenizer);
        op = parse_operand(parser, token, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                parser,
                "Argument %i Of Instruction '%.*s' Should Be A Register, Got '%.*s' Instead\n\n",
                op_count + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
            );
            return 1;
        }
        op1.as_uint8 = op.value.as_uint8;
        op_count += 1;
        break;
    case OP_PROFILE_RL:
        token = get_next_token(parser->tokenizer);
        op = parse_operand(parser, token, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                parser,
                "Argument %i Of Instruction '%.*s' Should Be A Register, Got '%.*s' Instead\n\n",
                op_count + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
            );
            return 1;
        }
        op2.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_L:
        token = get_next_token(parser->tokenizer);
        if(token.type == TKN_STR){
            if(inst.inst != INST_STATIC){
                REPORT_ERROR(
                    parser,
                    "Argument %i Of Instruction '%.*s' Can't Be A String\n\n",
                    op_count + 1, inst_token.size, inst_token.value.as_str
                );
                return 1;
            }
            op1.as_uint64 = static_memory->size;
            if(token.value.as_str[token.size - 1] != '\"'){
                REPORT_ERROR(parser, "Missing Closing %c\n\n", '\"');
                return 1;
            }
            mc_stream(static_memory, token.value.as_str + 1, token.size - 2);
            mc_stream_str(static_memory, "");
            break;
        }
        op = parse_operand(parser, token, hint);
        if(op.type != TKN_ULIT && op.type != TKN_ILIT && op.type != TKN_FLIT){
            REPORT_ERROR(
                parser,
                "Argument %i Of Instruction '%.*s' Should Be A Literal, Got '%.*s' Instead\n\n",
                op_count + 1, inst_token.size, inst_token.value.as_str, token.size, token.value.as_str
            );
            return 1;
        }
        op1.as_uint64 = op.value.as_uint64;
        break;
    
    default:
        break;
    }

    mc_stream(streambuff, &inst.inst, 1);
    
    switch (inst.profile)
    {
    case OP_PROFILE_RRR: mc_stream(streambuff, &op3.as_uint8, 1);
    case OP_PROFILE_RR:  mc_stream(streambuff, &op2.as_uint8, 1);
    case OP_PROFILE_R:   mc_stream(streambuff, &op1.as_uint8, 1); break;

    case OP_PROFILE_RL: mc_stream(streambuff, &op2.as_uint8, 1);
    case OP_PROFILE_L:  mc_stream(streambuff, &op1.as_uint64, 8); break;        
    
    default:
        break;
    }

    return 0;    
}

int parse_macro(Parser* parser, const Token macro, Token* include_path){

    fprintf(stderr, "TODO Parse Macro\n");
    exit(1);

    Token tokens[3];
    int argc = 0;//get_token_till(parser->tokenizer, tokens, "\n", 3);
    tokenizer_goto(parser->tokenizer, "\n");
    
    if(COMP_TKN(macro, MKTKN("%include"))){
        const Token arg = get_next_token(parser->tokenizer);
        if(arg.value.as_str == NULL){
            REPORT_ERROR(
                parser,
                "Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%include",
                0
            );
            return 1;
        }
        *include_path = arg;
        return 0;
    }
    else if(COMP_TKN(macro, MKTKN("%label"))){
        const Token arg1 = get_next_token(parser->tokenizer);
        if(arg1.type == TKN_NONE){
            REPORT_ERROR(parser, "Missing Label Identifier%c\n", ' ');
            return 1;
        }
        else if(argc > 2){
            REPORT_ERROR(parser, "To Many Arguments For %s Macro\n", "%label");
            return 1;
        }
        else if(argc == 1){
            add_label(parser->labels, tokens[0], MKTKN(""), HINT_NONE);
            return 0;
        }
        else{
            if(add_label(parser->labels, tokens[0], tokens[1], HINT_NONE)){
                REPORT_ERROR(
                    parser, "Invalid Label Or Definition '%s %.*s %.*s'\n",
                    "%label",
                    tokens[0].size, tokens[0].value.as_str, tokens[1].size, tokens[1].value.as_str
                );
                return 1;
            }
        }
        return 0;
    }
    else if(COMP_TKN(macro, MKTKN("%unlabel"))){
        if(argc < 1){
            REPORT_ERROR(parser, "Missing Label Identifier%c\n", ' ');
            return 1;
        }
        else if(argc > 1){
            REPORT_ERROR(parser, "To Many Arguments For %s Macro\n", "%unlabel");
            return 1;
        }
        if(remove_label(parser->labels, tokens[0])){
            REPORT_ERROR(parser, "Attempting To Unlabel '%.*s' While Label Does Not Exist\n", tokens[0].size, tokens[0].value.as_str);
            return 1;
        }
        return 0;
    }
    else if(COMP_TKN(macro, MKTKN("%iflabel"))){
        if(argc != 1){
            REPORT_ERROR(parser, "Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%iflabel", argc);
            return 1;
        }
        if(!get_label(parser->labels, tokens[0])){
            tokenizer_goto(parser->tokenizer, "%endif");
            get_next_token(parser->tokenizer);
        }
        return 0;
    }
    else if(COMP_TKN(macro, MKTKN("%ifnlabel"))){
        if(argc != 2){
            REPORT_ERROR(parser, "Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%iflabel", argc);
            return 1;
        }
        if(get_label(parser->labels, tokens[0])){
            tokenizer_goto(parser->tokenizer, "%endif");
            get_next_token(parser->tokenizer);
        }
        return 0;
    }

    return 0;
}

// \returns the relative path to the next file
int parse_file(Parser* parser, Mc_stream_t* program, Mc_stream_t* static_memory, const char* path){

    for(Token token = get_next_token(parser->tokenizer);
        (token.type != TKN_NONE);
        token = get_next_token(parser->tokenizer)){

        if(token.type == TKN_ERROR){
            return 1;
        }

        if(token.type == TKN_MACRO_INST){
            if(parse_macro(parser, token, NULL)){
                return 1;
            }
            continue;
        }
        {
            const Tokenizer pvr_tokenizer_state = *(parser->tokenizer);
            const Token next_token = get_next_token(parser->tokenizer);
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
                        "Could Not Add Label '%.*s', Label Is Either Invalid Or Has Already Been Defined\n",
                        token.size, token.value.as_str
                    );
                    return 1;
                }
                continue;
            }
            *(parser->tokenizer) = pvr_tokenizer_state;
        }
        
        parser->current_inst = get_inst_type(token);
        if(parser->current_inst.inst == INST_ERROR){
            REPORT_ERROR(parser, "Invalid Instruction '%.*s'\n\n", token.size, token.value.as_str);
            return 1;
        }

        if(parse_instruction(parser, token, program, static_memory)){
            return 1;
        }

    }

    return 0;
}


#endif