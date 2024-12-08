#ifndef VPU_LEXER_H
#define VPU_LEXER_H

#include "lexer.h"
#include <stdio.h>
#include "core.h"

#define COMP_STR(STR1, STR2) mc_compare_str(STR1, STR2, 0)

#define REPORT_ERROR(STR, ...) fprintf(stderr, "[ERROR] In File '%s', On Line %zu:\n\t" STR, file_path.data, line, __VA_ARGS__)

#define REPORT_WARNING(STR, ...) fprintf(stderr, "[WARNING] In File '%s', On Line %zu:\n\t" STR, file_path.data, line, __VA_ARGS__)

#define REP_INVALID_REGID(TOKEN) REPORT_ERROR("[ERROR] Invalid Register Identifier '%s', No Such Register\n", TOKEN)

static Mc_stream_t file_path;
static size_t line = 1;
static int flags = 0;
static int error_count  = 0;

static uint64_t entry_point = 0;

static int expexted_op1 = 0;
static int expected_op2 = 0;

enum ArgFlags{

    FLAG_NONE = 0,

    FLAG_EXPECT_INST = 1 << 0,
    FLAG_EXPECT_REG  = 1 << 1,
    FLAG_EXPECT_LIT  = 1 << 2,

    FLAG_ERROR = 1 << 3,
    FLAG_TEST  = 1 << 4,

    FLAG_ADD_SYM_TABLE = 1 << 5
};

enum TokenTypes{

    TKN_NONE  = 0,
    TKN_INST  = 1,
    TKN_REG   = 1 << 1,
    TKN_LIT   = 1 << 2,
    TKN_INT   = 1 << 3,
    TKN_UINT  = 1 << 4,
    TKN_FLOAT = 1 << 5,
    TKN_LABEL = 1 << 6

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
    int type;

} Operand;

typedef struct InstProfile{

    uint8_t   inst;
    OpProfile profile;

} InstProfile;


int get_digit(char c){
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;

    default: return -1;
    }
}

char get_char_digit(int d){
    switch (d)
    {
    case 0: return '0';
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

uint8_t get_hex_digit(char c){
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;

    default: return 255;
    }
}

InstProfile get_inst_type(const char* inst_str){

    if(COMP_STR(inst_str, "NOP"))    return (InstProfile){INST_NOP   , OP_PROFILE_NONE};
    if(COMP_STR(inst_str, "HALT"))   return (InstProfile){INST_HALT  , OP_PROFILE_NONE};
    if(COMP_STR(inst_str, "MOV8"))   return (InstProfile){INST_MOV8  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MOV16"))  return (InstProfile){INST_MOV16 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MOV32"))  return (InstProfile){INST_MOV32 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MOV"))    return (InstProfile){INST_MOV   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MOVV"))   return (InstProfile){INST_MOVV  , OP_PROFILE_RL};
    if(COMP_STR(inst_str, "PUSH"))   return (InstProfile){INST_PUSH  , OP_PROFILE_R};
    if(COMP_STR(inst_str, "PUSHV"))  return (InstProfile){INST_PUSHV , OP_PROFILE_L};
    if(COMP_STR(inst_str, "POP"))    return (InstProfile){INST_POP   , OP_PROFILE_R};
    if(COMP_STR(inst_str, "GET"))    return (InstProfile){INST_GET   , OP_PROFILE_RL};
    if(COMP_STR(inst_str, "WRITE"))  return (InstProfile){INST_WRITE , OP_PROFILE_RL};
    if(COMP_STR(inst_str, "GSP"))    return (InstProfile){INST_GSP   , OP_PROFILE_R};
    if(COMP_STR(inst_str, "STATIC")) return (InstProfile){INST_STATIC, OP_PROFILE_L};
    if(COMP_STR(inst_str, "READ8"))  return (InstProfile){INST_READ8 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "READ16")) return (InstProfile){INST_READ16, OP_PROFILE_RR};
    if(COMP_STR(inst_str, "READ32")) return (InstProfile){INST_READ32, OP_PROFILE_RR};
    if(COMP_STR(inst_str, "READ"))   return (InstProfile){INST_READ  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SET8"))   return (InstProfile){INST_SET8  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SET16"))  return (InstProfile){INST_SET16 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SET32"))  return (InstProfile){INST_SET32 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SET"))    return (InstProfile){INST_SET   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "TEST"))   return (InstProfile){INST_TEST  , OP_PROFILE_R};
    if(COMP_STR(inst_str, "NOT"))    return (InstProfile){INST_NOT   , OP_PROFILE_R};
    if(COMP_STR(inst_str, "NEG"))    return (InstProfile){INST_NEG   , OP_PROFILE_R};
    if(COMP_STR(inst_str, "AND"))    return (InstProfile){INST_AND   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "NAND"))   return (InstProfile){INST_NAND  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "OR"))     return (InstProfile){INST_OR    , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "XOR"))    return (InstProfile){INST_XOR   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "BSHIFT")) return (InstProfile){INST_BSHIFT, OP_PROFILE_RR};
    if(COMP_STR(inst_str, "JMP"))    return (InstProfile){INST_JMP   , OP_PROFILE_L};
    if(COMP_STR(inst_str, "JMPF"))   return (InstProfile){INST_JMPIF , OP_PROFILE_RL};
    if(COMP_STR(inst_str, "JMPFN"))  return (InstProfile){INST_JMPIFN, OP_PROFILE_RL};
    if(COMP_STR(inst_str, "CALL"))   return (InstProfile){INST_JMP   , OP_PROFILE_R};
    if(COMP_STR(inst_str, "RET"))    return (InstProfile){INST_JMPIF , OP_PROFILE_NONE};
    if(COMP_STR(inst_str, "ADD8"))   return (InstProfile){INST_ADD8  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SUB8"))   return (InstProfile){INST_SUB8  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MUL8"))   return (InstProfile){INST_MUL8  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "ADD16"))  return (InstProfile){INST_ADD16 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SUB16"))  return (InstProfile){INST_SUB16 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MUL16"))  return (InstProfile){INST_MUL16 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "ADD32"))  return (InstProfile){INST_ADD32 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SUB32"))  return (InstProfile){INST_SUB32 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MUL32"))  return (InstProfile){INST_MUL32 , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "ADD"))    return (InstProfile){INST_ADD   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SUB"))    return (InstProfile){INST_SUB   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MUL"))    return (InstProfile){INST_MUL   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "DIVI"))   return (InstProfile){INST_DIVI  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "DIVU"))   return (InstProfile){INST_DIVU  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "ADDF"))   return (InstProfile){INST_ADDF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SUBF"))   return (InstProfile){INST_SUBF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "MULF"))   return (InstProfile){INST_MULF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "DIVF"))   return (InstProfile){INST_DIVF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "EQI"))    return (InstProfile){INST_EQI   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "EQU"))    return (InstProfile){INST_EQU   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "EQF"))    return (InstProfile){INST_EQF   , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "BIGI"))   return (InstProfile){INST_BIGI  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "BIGU"))   return (InstProfile){INST_BIGU  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "BIGF"))   return (InstProfile){INST_BIGF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SMLI"))   return (InstProfile){INST_SMLI  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SMLU"))   return (InstProfile){INST_SMLU  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "SMLF"))   return (InstProfile){INST_SMLF  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "CASTIU")) return (InstProfile){INST_CASTIU, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CASTIF")) return (InstProfile){INST_CASTIF, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CASTUI")) return (InstProfile){INST_CASTUI, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CASTUF")) return (InstProfile){INST_CASTUF, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CASTFI")) return (InstProfile){INST_CASTFI, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CASTFU")) return (InstProfile){INST_CASTFU, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CF3264")) return (InstProfile){INST_CF3264, OP_PROFILE_R};
    if(COMP_STR(inst_str, "CF6432")) return (InstProfile){INST_CF6432, OP_PROFILE_R};
    if(COMP_STR(inst_str, "MEMSET")) return (InstProfile){INST_MEMSET, OP_PROFILE_RRR};
    if(COMP_STR(inst_str, "MEMCPY")) return (InstProfile){INST_MEMCPY, OP_PROFILE_RRR};
    if(COMP_STR(inst_str, "MEMMOV")) return (InstProfile){INST_MEMMOV, OP_PROFILE_RRR};
    if(COMP_STR(inst_str, "MEMCMP")) return (InstProfile){INST_MEMCMP, OP_PROFILE_RRR};
    if(COMP_STR(inst_str, "MALLOC")) return (InstProfile){INST_MALLOC, OP_PROFILE_R};
    if(COMP_STR(inst_str, "FREE"))   return (InstProfile){INST_FREE  , OP_PROFILE_R};
    if(COMP_STR(inst_str, "FOPEN"))  return (InstProfile){INST_FOPEN , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "FCLOSE")) return (InstProfile){INST_FCLOSE, OP_PROFILE_R};
    if(COMP_STR(inst_str, "PUTC"))   return (InstProfile){INST_PUTC  , OP_PROFILE_RR};
    if(COMP_STR(inst_str, "GETC"))   return (InstProfile){INST_GETC  , OP_PROFILE_R};

    if(COMP_STR(inst_str, "SYS"))    return (InstProfile){INST_SYS   , OP_PROFILE_L};
    if(COMP_STR(inst_str, "DISREG")) return (InstProfile){INST_DISREG, OP_PROFILE_R};

    return (InstProfile){INST_ERROR, 0};
}

uint64_t parse_hexadecimal(const char* str){

    uint64_t n = 0;
    
    for(size_t i = 0; str[i]; i+=1){
        const uint8_t digit = get_hex_digit(str[i]);

        if(digit == 255){
            flags |= FLAG_TEST;
            return 0;
        }

        n = (n << 4) | digit;
    }

    return n;
}

uint64_t parse_uint(const char* str){
    size_t size = 0;
    for(; str[size]; size+=1);

    uint64_t _10n = 1;
    uint64_t output = 0;
    
    for(int i = size - 1; i > -1; i-=1){

        const int digit = get_digit(str[i]);

        if(digit < 0){
            flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    return output;
}

double parse_float(const char* str){
    size_t size = 0;
    int dot_pos = -1;
    for(; str[size]; size+=1){
        if(str[size] == '.'){
            if(dot_pos != -1){
                flags |= FLAG_TEST;
                return 0;
            }
            dot_pos = size;
        } else if(str[size] == 'f'){
            if(str[size + 1]){
                flags |= FLAG_TEST;
                return 0;
            }
            break;
        }
    }

    uint64_t _10n = 1;
    double output = 0;
    
    for(int i = size - 1; i > dot_pos; i-=1){

        const int digit = get_digit(str[i]);

        if(digit < 0){
            flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    if(dot_pos < 0) return output;

    output /= _10n;
    _10n = 1;

    for(int i = dot_pos - 1; i > -1; i-=1){
        const int digit = get_digit(str[i]);

        if(digit < 0){
            fprintf(stderr, "[ERROR] In Line %zu:\n\tInvalid Digit '%c'\n", line, str[i]);
            flags |= FLAG_TEST;
            return 0;
        }

        output += digit * _10n;
        _10n *= 10;
    }

    return output;
}

Operand parse_operand(const char* token, int hint){

    if(token[0] == '0'){
        if(token[1] == 'x' || token[1] == 'X'){
            const uint64_t hex = parse_hexadecimal(token + 2);
            if(flags & FLAG_TEST){
                REPORT_ERROR("Invalid Token '%s'\n", token);
                flags &= ~FLAG_TEST;
                error_count += 1;
                return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
            }
            return (Operand){.value.as_uint64 = hex, .type = TKN_LIT | TKN_UINT};
        }
    }

    if(mc_compare_str(token, "RA", 1))  {
        if(token[2] == '\0'){
            return (Operand){.value.as_uint8 = RA,  .type = TKN_REG};    
        }
        const int i = get_digit(token[2]);
        if((i < 0) || token[3]){
            fprintf(stderr, "[ERROR] Invalid Register Identifier '%s', No Such Register\n", token);
            error_count += 1;
            return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        }
        return (Operand){.value.as_uint8 = RA + i,  .type = TKN_REG};
    }
    if(mc_compare_str(token, "RB", 1))  {
        if(token[2] == '\0'){
            return (Operand){.value.as_uint8 = RB,  .type = TKN_REG};    
        }
        const int i = get_digit(token[2]);
        if((i < 0) || token[3]){
            REP_INVALID_REGID(token);
            error_count += 1;
            return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        }
        return (Operand){.value.as_uint8 = RB + i,  .type = TKN_REG};
    }
    if(mc_compare_str(token, "RC", 1))  {
        if(token[2] == '\0'){
            return (Operand){.value.as_uint8 = RC,  .type = TKN_REG};    
        }
        const int i = get_digit(token[2]);
        if((i < 0) || token[3]){
            REP_INVALID_REGID(token);
            error_count += 1;
            return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        }
        return (Operand){.value.as_uint8 = RC + i,  .type = TKN_REG};
    }
    if(mc_compare_str(token, "RD", 1))  {
        if(token[2] == '\0'){
            return (Operand){.value.as_uint8 = RD,  .type = TKN_REG};    
        }
        const int i = get_digit(token[2]);
        if((i < 0) || token[3]){
            REP_INVALID_REGID(token);
            error_count += 1;
            return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        }
        return (Operand){.value.as_uint8 = RD + i,  .type = TKN_REG};
    }
    if(mc_compare_str(token, "RE", 1))  {
        if(token[2] == '\0'){
            return (Operand){.value.as_uint8 = RE,  .type = TKN_REG};    
        }
        const int i = get_digit(token[2]);
        if((i < 0) || token[3]){
            REP_INVALID_REGID(token);
            error_count += 1;
            return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        }
        return (Operand){.value.as_uint8 = RE + i,  .type = TKN_REG};
    }
    if(COMP_STR(token, "RF"))  return (Operand){.value.as_uint8 = RF,  .type = TKN_REG};
    if(COMP_STR(token, "RSP")) return (Operand){.value.as_uint8 = RSP, .type = TKN_REG};
    if(COMP_STR(token, "RIP")) return (Operand){.value.as_uint8 = RIP, .type = TKN_REG};

    int is_float = 0;

    for(size_t i = 0; token[i] && !is_float; i+=1){
        is_float = (token[i] == '.');
    }

    Register v;
    int type = TKN_NONE;
    const int i = (token[0] == '-');

    if(is_float){
        if(hint == HINT_NONE)       v.as_float64 = (double)(parse_float(token + i) * (1 - 2 * i));
        else if(hint == HINT_32BIT) v.as_float32 = (float )(parse_float(token + i) * (1 - 2 * i));
        else                        return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
        type = TKN_LIT | TKN_FLOAT;
    }
    else if(token[0] == '-'){
        switch (hint)
        {
        case HINT_8BIT : v.as_int8  = (int8_t )(-parse_uint(token + 1)); break;
        case HINT_16BIT: v.as_int16 = (int16_t)(-parse_uint(token + 1)); break;
        case HINT_32BIT: v.as_int32 = (int32_t)(-parse_uint(token + 1)); break;
        default:         v.as_int64 = (int64_t)(-parse_uint(token + 1)); break;
        }
        type = TKN_LIT | TKN_INT;
    }
    else{
        switch (hint)
        {
        case HINT_8BIT : v.as_uint8  = (uint8_t )(parse_uint(token)); break;
        case HINT_16BIT: v.as_uint16 = (uint16_t)(parse_uint(token)); break;
        case HINT_32BIT: v.as_uint32 = (uint32_t)(parse_uint(token)); break;
        default:         v.as_uint64 = (uint64_t)(parse_uint(token)); break;
        }
        type = TKN_LIT | TKN_UINT;
    }

    if(!(flags & FLAG_TEST)) 
        return (Operand){.value = v, .type = type};

    flags &= ~FLAG_TEST;
    
    return (Operand){.value.as_uint64 = 0, .type = TKN_NONE};
}

// \return the number of tokens read 
int parse_instruction(Mc_stream_t* streambuff, Mc_stream_t* static_memory, char** tokens, int argc){

    if(error_count > 50){
        fprintf(stderr, "[ERROR] Too Many Error Messages\n");
        exit(1);
    }

    int current_token = 0;

    char* token_str = tokens[current_token++];

    // while the token is just a new line character keep moving forward and incrementing the line count
    // (the tokenizer garanties that if the token's first character is \n then the next character is \0)
    //while(token_str[0] == '\n'){
    //    flags |= FLAG_EXPECT_INST;
    //    line += 1;
    //    current_token += 1;
    //    if(current_token == token_count) return skip + 1;
    //    skip +=1;
    //    token_str = mc_get_token_data(tokens, tokens[current_token]);
    //}

    InstProfile inst = get_inst_type(token_str);

    if(inst.inst == INST_ERROR){
        REPORT_ERROR("Expected Instruction, Got '%s' Instead\n\n", token_str);
        error_count += 1;
        return 1;
    }

    const int hint =
        (inst.inst == INST_MOV8 ) * HINT_8BIT  +
        (inst.inst == INST_MOV16) * HINT_32BIT +
        (inst.inst == INST_MOV32) * INST_ADD32;

    Register op1;
    Register op2;
    Register op3;
    Operand op;

    const char* const inst_str = token_str;

    int op_count = 0;

    switch (inst.profile)
    {
    case OP_PROFILE_RRR:
        token_str = tokens[current_token++];
        op = parse_operand(token_str, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                "Argument %i Of Instruction '%s' Should Be A Register, Got '%s' Instead\n\n",
                op_count + 1, inst_str, token_str
            );
            error_count += 1;
            break;
        }
        op2.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_RR:
        token_str = tokens[current_token++];
        op = parse_operand(token_str, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                "Argument %i Of Instruction '%s' Should Be A Register, Got '%s' Instead\n\n",
                op_count + 1, inst_str, token_str
            );
            error_count += 1;
            break;
        }
        op2.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_R:
        token_str = tokens[current_token++];
        op = parse_operand(token_str, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                "Argument %i Of Instruction '%s' Should Be A Register, Got '%s' Instead\n\n",
                op_count + 1, inst_str, token_str
            );
            error_count += 1;
        }
        op1.as_uint8 = op.value.as_uint8;
        op_count += 1;
        break;
    case OP_PROFILE_RL:
        token_str = tokens[current_token++];
        op = parse_operand(token_str, HINT_NONE);
        if(op.type != TKN_REG){
            REPORT_ERROR(
                "Argument %i Of Instruction '%s' Should Be A Register, Got '%s' Instead\n\n",
                op_count + 1, inst_str, token_str
            );
            error_count += 1;
            break;
        }
        op2.as_uint8 = op.value.as_uint8;
        op_count += 1;
    case OP_PROFILE_L:
        token_str = tokens[current_token++];
        if(token_str[0] == '\"'){
            if(inst.inst != INST_STATIC){
                REPORT_ERROR(
                    "Argument %i Of Instruction '%s' Can't Be A String\n\n",
                    op_count + 1, inst_str
                );
                error_count += 1;
                op1.as_uint64 = 0;
                break;
            }
            op1.as_uint64 = static_memory->size;
            char* opstr = token_str + 1;
            for(size_t i = 0; opstr[i]; i++){
                if(opstr[i] == '\"'){
                    opstr[i] = '\0';
                    break;
                }
            }
            mc_stream_str(static_memory, opstr);
            op_count += 1;
            break;
        }
        op = parse_operand(token_str, hint);
        if(!(op.type & TKN_LIT)){
            REPORT_ERROR(
                "Argument %i Of Instruction '%s' Should Be A Literal, Got '%s' Instead\n\n",
                op_count + 1, inst_str, token_str
            );
            error_count += 1;
        }
        op1.as_uint64 = op.value.as_uint64;
        op_count += 1;
        break;
    
    default:
        break;
    }

    if(0 == error_count){
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
    }

    return 1 + op_count;    
}

int remove_label(Mc_stream_t* labels, Mc_stream_t* label_definitions, const char* label){

    for(size_t i = 0; i < labels->size; i+=8){

        const char* lstr = (labels->data + i + 8);
        if(COMP_STR(lstr, label)){
            size_t lsize = 0; for(; lstr[lsize]; lsize+=1); lsize += 1;
            memmove(labels->data + i, labels->data + i + lsize + 8, labels->size - i - lsize - 8);
            labels->size -= lsize + 8;

            const uint64_t defp = *(uint64_t*)(lstr - 8);
            const char* dstr = label_definitions->data + defp;
            size_t dsize = 0; for(; dstr[dsize]; dsize+=1); dsize += 1;
            memmove(
                label_definitions->data + defp,
                label_definitions->data + defp + dsize,
                label_definitions->size - defp - dsize
            );
            label_definitions->size -= dsize;
            return 0;
        }
        while (lstr[i++]);
    }

    return 1;
}

int add_label(Mc_stream_t* labels, Mc_stream_t* label_definitions, const char* label, const char* definition, int hint){

    if(label[0] == '\"' || label[0] == '\0') return 1;

    for(size_t i = 0; i < labels->size; i+=8){

        const char* lstr = (labels->data + i + 8);
        if(COMP_STR(lstr, label)){
            const uint64_t defp = *(uint64_t*)(lstr - 8);
            REPORT_WARNING("Label '%s' Redefined From '%s' To '%s'\n", label,label_definitions->data + defp , definition);

            const char* dstr = label_definitions->data + defp;
            size_t dsize = 0; for(; dstr[dsize]; dsize+=1); dsize += 1;
            memmove(
                label_definitions->data + defp,
                label_definitions->data + defp + dsize,
                label_definitions->size - defp - dsize
            );
            size_t new_def_size = 0;
            while (definition[new_def_size++]);
            
            label_definitions->size = label_definitions->size + (int)(new_def_size - dsize);
            *(uint64_t*)(lstr - 8) = label_definitions->size;
            mc_stream_str(label_definitions, definition);
            break;
        }
        while (lstr[i++]);
    }

    const char nullterm_c = '\0';

    mc_stream(labels, &label_definitions->size, sizeof(uint64_t));
    mc_stream_str(labels, label);

    mc_stream_str(label_definitions, definition);

    return 0;
}

// the returned pointer is only valid up to the next use of add_label or remove_label
char* get_label(Mc_stream_t* labels, Mc_stream_t* label_definitions, const char* label){

    if(label[0] == '\"' || label[0] == '\0') return NULL;

    for(size_t i = 0; i < labels->size; i+=8){

        const char* lstr = (labels->data + i + 8);
        if(COMP_STR(lstr, label)){
            const uint64_t defp = *(uint64_t*)(lstr - 8);
            return label_definitions->data + defp;
        }
        while (lstr[i++]);        
    }

    return NULL;
}

int traslate_labels(char** output, Mc_stream_t *labels, Mc_stream_t *label_definitions, Mc_token_t* tokens, size_t* current_token, size_t token_count){

    int argc = 0;

    for (char* token_str = "\0";
        token_str[0] != '\n' && *current_token < token_count;
        *current_token += 1
    ) {
        token_str = mc_get_token_data(tokens, tokens[*current_token]);
        if(token_str[0] == '\n') break;
        char* arg = get_label(labels, label_definitions, token_str);
        if(arg){
            output[argc++] = arg;
        }
        else{
            output[argc++] = token_str;
        }
    }

    return argc;
}

int parse_macro(Mc_stream_t* labels, Mc_stream_t* label_definitions, Mc_token_t* tokens, size_t current_token, size_t token_count, char** include_path){

    int skip = 0;

    while (current_token + skip < token_count && mc_get_token_data(tokens, tokens[current_token + skip])[0] != '\n')
    {
        skip += 1;
    }
    

    const char* macro = mc_get_token_data(tokens, tokens[current_token]);

    *include_path = NULL;

    if(COMP_STR(macro, "%include")){
        if(skip != 2){
            REPORT_ERROR("Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%include", skip - 1);
            error_count += 1;
            flags |= FLAG_ERROR;
            return skip;
        }
        char* const ipstr = mc_get_token_data(tokens, tokens[current_token + 1]) + 1;
        size_t i = 0;
        for(; ipstr[i] && (ipstr[i] != '\"'); i+=1);
        ipstr[i] = '\0';
        *include_path = ipstr;
        return skip;
    }
    else if(COMP_STR(macro, "%label")){
        if(skip < 2){
            REPORT_ERROR("Missing Label Identifier%c\n", ' ');
            error_count += 1;
            return skip;
        }
        else if(skip > 3){
            REPORT_ERROR("To Many Arguments For %s Macro\n", "%label");
            error_count += 1;
            return skip;
        }
        else if(skip == 2){
            const char* label = mc_get_token_data(tokens, tokens[current_token + 1]);
            add_label(labels, label_definitions, label, "", HINT_NONE);
            return skip;
        }
        else{
            const char* label = mc_get_token_data(tokens, tokens[current_token + 1]);
            const char* definition = mc_get_token_data(tokens, tokens[current_token + 2]);
            if(add_label(labels, label_definitions, label, definition, HINT_NONE)){
                REPORT_ERROR("Invalid Label Or Definition '%s %s %s'\n", "%label", label, definition);
                error_count += 1;
            }
            return skip;
        }
        return skip;
    }
    else if(COMP_STR(macro, "%unlabel")){
        if(skip < 2){
            REPORT_ERROR("Missing Label Identifier%c\n", ' ');
            error_count += 1;
            return skip;
        }
        else if(skip > 2){
            REPORT_ERROR("To Many Arguments For %s Macro\n", "%unlabel");
            error_count += 1;
            return skip;
        }
        const char* label = mc_get_token_data(tokens, tokens[current_token + 1]);
        if(remove_label(labels, label_definitions, label)){
            REPORT_ERROR("Attempting To Unlabel '%s' While Label Does Not Exist\n", label);
            error_count += 1;
            return skip;
        }
    }
    else if(COMP_STR(macro, "%iflabel")){
        if(skip != 2){
            REPORT_ERROR("Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%iflabel", skip - 1);
            error_count += 1;
            flags |= FLAG_ERROR;
            return skip;
        }
        if(!get_label(labels, label_definitions, mc_get_token_data(tokens, tokens[current_token + 1]))){
            for(current_token += skip; current_token < token_count; current_token += 1){
                if(COMP_STR(macro, "%endif")) break;
                skip += 1;
            }
            return skip;
        }
        return skip;
    }
    else if(COMP_STR(macro, "%ifnlabel")){
        if(skip != 2){
            REPORT_ERROR("Incorrect Usage For %s Macro, Expected 1 Argument Got %i Instead\n", "%iflabel", skip - 1);
            error_count += 1;
            flags |= FLAG_ERROR;
            return skip;
        }
        if(get_label(labels, label_definitions, mc_get_token_data(tokens, tokens[current_token + 1]))){
            for(current_token += skip; current_token < token_count; current_token += 1){
                if(COMP_STR(macro, "%endif")) break;
                skip += 1;
            }
            return skip;
        }
        return skip;
    }

    return skip;
}

int parse_file_recusive(Mc_stream_t* program, Mc_stream_t* static_memory, Mc_stream_t* labels,
    Mc_stream_t* labels_definitions, const char* relative_path){

    static int         include_depth = 0;
    static Mc_token_t* token_buffer  = NULL;
    static size_t      token_count   = 0;
    static size_t      current_token = 0;
    static char*       argv[5]          ;
    
    size_t file_line = 0;
    line = file_line;

    if(!include_depth){
        file_path = mc_create_stream(1000);
        token_buffer = mc_create_token_buffer(100, 1000);
        printf("[INFO] Recursive Parsing Stream Created\n");
    }
    else if(include_depth > 100){
        REPORT_ERROR("Maximum Include Depth Exceeded %c\n", ' ');
        error_count += 1;
        return 1;
    }


    int mother_dir_len = (int)file_path.size;
    for (; (mother_dir_len > -1) && (file_path.data[mother_dir_len] != '/'); mother_dir_len-=1);
    mother_dir_len += 1;

    const size_t previous_program_size = program->size;

    {
        size_t relative_path_size = 0;
        while(relative_path[relative_path_size++]);
        file_path.size += relative_path_size;
        if(file_path.size >= file_path.capacity){
            mc_change_stream_cap(&file_path, file_path.capacity * (size_t)(1 + (file_path.size / file_path.capacity)));
        }
        memmove(file_path.data + mother_dir_len + relative_path_size, file_path.data + mother_dir_len, file_path.size - relative_path_size - mother_dir_len);
        file_path.size = mother_dir_len;
        mc_stream(&file_path, relative_path, relative_path_size);

        if(!read_file(program, file_path.data, "r")){
            fprintf(
                stderr,
                "[ERROR] In File '%.*s%s', On Line %zu\n\t"
                "Unable To Open/Read File '%s', Given As '%s' In Directory '%.*s'\n",
                mother_dir_len, file_path.data, file_path.data + mother_dir_len + relative_path_size, line,
                file_path.data, relative_path, mother_dir_len, file_path.data
            );
            error_count += 1;
            if(!include_depth){
                mc_destroy_stream(file_path);
                mc_destroy_token_buffer(token_buffer);
                printf("[INFO] Recursive Parsing Stream Destroyed\n");
            }
            return 1;
        }

    }

    program->size = 0;

    token_count += mc_tokenize(
        &token_buffer,
        token_count,
        program->data + previous_program_size,
        " \t", ":\n", ";", '\"'
    );

    char* next_path = NULL;

    while(current_token < token_count){
        const char* tkn_str = mc_get_token_data(token_buffer, token_buffer[current_token]);
        while(tkn_str[0] == '\n' && current_token < token_count){
            line += 1;
            current_token += 1;
            tkn_str = mc_get_token_data(token_buffer, token_buffer[current_token]);
        }
        if(current_token == token_count) break;
        if(tkn_str[0] == '%'){
            current_token += parse_macro(labels, labels_definitions, token_buffer, current_token, token_count, &next_path);
            if(next_path){
                file_line = line;
                size_t prv_token_count = token_count;
                size_t prv_token = current_token;
                current_token = token_count;
                include_depth += 1;
                if(parse_file_recusive(program, static_memory, labels, labels_definitions, next_path)){
                    if(!include_depth){
                        mc_destroy_stream(file_path);
                        mc_destroy_token_buffer(token_buffer);
                        printf("[INFO] Recursive Parsing Stream Destroyed\n");
                    }
                    return 1;
                }
                include_depth -= 1;
                
                line = file_line;
                token_count = prv_token_count;
                current_token = prv_token;
                file_path.size = mother_dir_len;
                mc_stream_str(&file_path, relative_path);
            }
            continue;
        } else if(current_token + 1 < token_count){
            const char* const tkn2_str = mc_get_token_data(token_buffer, token_buffer[current_token + 1]);
            if(tkn2_str[0] == ':'){
                uint64_t ss = program->size;
                char buffer[25];
                int digits = 1;
                for(; ss/10; ss/=10){
                    digits += 1;
                }
                ss = program->size;
                buffer[digits] = '\0';
                for(int d = digits - 1; d > -1; d-=1){
                    const int digit = ss - 10 * (uint64_t)(ss / 10);
                    buffer[d] = get_char_digit(digit);
                    ss /= 10;
                }
                if(add_label(labels, labels_definitions, tkn_str, buffer, HINT_NONE)){
                    REPORT_ERROR("Bad Label '%s'\n", tkn_str);
                    error_count += 1;
                    continue;
                }
                current_token += 2;
                continue;
            }
        }
        int argc = traslate_labels(argv, labels, labels_definitions, token_buffer, &current_token, token_count);
        parse_instruction(program, static_memory, argv, argc);
    }

    if(!include_depth){
        mc_destroy_stream(file_path);
        mc_destroy_token_buffer(token_buffer);
        printf("[INFO] Recursive Parsing Stream Destroyed\n");
    }


    return 0;
}


#endif