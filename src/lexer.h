#ifndef VIRTUAL_LEXER_H
#define VIRTUAL_LEXER_H


#include "core.h"
#include "virtual.h"



typedef struct Tokenizer
{
    char*    data;
    uint64_t pos;
    int      line;
    int      column;
} Tokenizer;

typedef union TokenValue{
    char*    as_str;
    uint64_t as_uint;
    int64_t  as_int;
    double   as_float;
    char     as_char;
} TokenValue;

typedef struct Token
{
    TokenValue  value;
    int         size;
    //int         line;
    //int         column;
    uint8_t     type;
} Token;


enum TokenTypes{
    TKN_NONE = 0,
    TKN_RAW,
    TKN_INST,
    TKN_REG,
    TKN_NUM,
    TKN_ILIT,
    TKN_ULIT,
    TKN_FLIT,
    TKN_STR,
    TKN_CHAR,
    TKN_SPECIAL_SYM,
    TKN_MACRO_INST,
    TKN_LABEL_REF,
    TKN_EMPTY,
    TKN_ADDR_LABEL_REF,
    TKN_STATIC_SIZE,
    TKN_UNRESOLVED_LABEL,
    TKN_INST_POSITION,
    TKN_ENDEXPORT,
    TKN_ERROR = 255,
};

typedef struct StringView
{
    char*    str;
    uint32_t size;
} StringView;

typedef struct LexizedString{
    char* str;
    int   read;
    int   written;
} LexizedString;


#define MKTKN(STR) ((Token){.value.as_str = STR, .size = sizeof(STR) - 1, .type = TKN_RAW})

char get_digit_char(int i);

Token get_token_from_cstr(const char* str);

void tokenizer_goto(Tokenizer* tokenizer, const char* dest);

int mc_compare_token(const Token token1, const Token token2, int _only_compare_till_smaller);

LexizedString lexize_str(char* str, char delim);

Token get_next_token(Tokenizer* tokenizer);

// if include_file_path is NOT 0 then, on success, the file path will be streamed to the stream as streamview
// (first size (uint32) then cstr (null terminated)) before the file contents
char* read_file_txt(Mc_stream_t* stream, const char* path, int include_file_path);

// this automatically includes the concatonated file path as stringview (first size (uint32) then cstr (null terminated))
// to the stream before the file contents, only if on success
char* read_file_relative(Mc_stream_t* stream, StringView mother_dir, StringView relative_path);

#endif
