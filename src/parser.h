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


void fprint_token(FILE* file, const Token token);

const char* get_token_type_str(int type);

uint8_t get_inst_op(const Token inst_token);

OpProfile get_inst_profile(int inst);

int get_reg(const Token token);

Operand parse_op_literal(Token token);

int push_to_static(Mc_stream_t* static_memory, const Token token);

int parse_macro(Parser* parser, const Token macro, StringView* include_path);

int pre_parse_inst_operand(const Parser* parser, Token* _token, uint64_t absolute_program_position);

// \returns the parsed instruction on success or INST_ERROR on failure
Inst parse_inst(Parser* parser, InstProfile inst_profile, const StringView inst_sv);

// \return 1 on error or 0 otherwise
int parse_file(Parser* parser, Mc_stream_t* files_stream);


#endif // =====================  END OF FILE VIRTUAL_PARSER_H ===========================