#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "begin.h"
#include <stdlib.h>

enum TokenType{
    TKNTYPE_ERROR = -1,
    TKNTYPE_NONE = 0,
    TKNTYPE_CHAR,
    TKNTYPE_UCHAR,
    TKNTYPE_INT,
    TKNTYPE_UINT,
    TKNTYPE_FLOAT,
    TKNTYPE_DOUBLE,
    TKNTYPE_PTR,
    TKNTYPE_STR,
    TKNTYPE_RAW,

    TKNTYPE_OPENPAREN   = '(',
    TKNTYPE_CLOSEPAREN  = ')',
    TKNTYPE_OPENCURLY   = '{',
    TKNTYPE_CLOSECURLY  = '}',
    TKNTYPE_COMMA       = ',',
    TKNTYPE_DOT         = '.',
    TKNTYPE_SEMICOLUMN  = ';',
    TKNTYPE_COLUMN      = ':',

    TKNTYPE_XOR         = 256,
    TKNTYPE_SL,
    TKNTYPE_SR,
    TKNTYPE_PP,
    TKNTYPE_MM,
    TKNTYPE_OO,
    TKNTYPE_AA,
    TKNTYPE_EE,
    TKNTYPE_GE,
    TKNTYPE_LE,
    TKNTYPE_PE,
    TKNTYPE_ME,
    TKNTYPE_TE,
    TKNTYPE_DE,
    TKNTYPE_XE,
    TKNTYPE_AE,
    TKNTYPE_OE,
    TKNTYPE_OOE,
    TKNTYPE_AAE,
    TKNTYPE_SLE,
    TKNTYPE_SRE,
    TKNTYPE_NE,

};

typedef struct Token
{
    int type;
    TokenValue       value;

    const char* file;
    int         line;
    int         column;
} Token;

typedef struct Tokenizer
{
    const char* src;
    const char* src_file_name;
    size_t  pos;
    int     line;
    int     column;
    Token   tkn_buff[10];
    int     tkn_buff_size;
} Tokenizer;

int special_character(char c);


const char* get_tkntype_str(int tkntype);

int is_token_literal(int tkntype);

int is_token_operand(int tkntype);

int is_token_assign(int tkntype);

int print_token(const Token token);

Tokenizer create_tokenizer(const char* src, int pos, const char* src_file_name);

int tokenize_cstr_numeric(TokenValue* value, const char* cstr, int* len);

int tokenize_cstr(TokenValue* value, const char* cstr, int* len);


Token next_token();

Token peek(int count);

Token skip(int count);

Token expect(int tkn_type);

Token expect_cstr(const char* cstr);


extern Tokenizer tokenizer;


#endif // =====================  END OF FILE TOKENIZER_H ===========================