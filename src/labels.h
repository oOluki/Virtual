#ifndef VIRTUAL_LABLES_H
#define VIRTUAL_LABLES_H

#include "virtual.h"
#include "lexer.h"

enum LabelFlags{
    LABELFLAG_NONE      = 0,
    LABELFLAG_RESOLVED  = 1 << 0,
    LABELFLAG_EXPORT    = 1 << 1,
    LABELFLAG_IMPORT    = 1 << 2
};

enum LabelerFlags{
    LABELERFLAG_NONE                = 0,
    LABELERFLAG_LOCAL_LABEL_SUPPORT = 1 << 0,
};

typedef struct Label
{
    uint32_t   size;
    uint32_t   str;
    uint32_t   str_size;
    uint8_t    type;
    uint8_t    flags;
    uint64_t   inst_position;
    TokenValue definition;
} Label;

typedef struct Labeler
{
    Mc_stream_t labels;
    Mc_stream_t local_labels;
    uint8_t     flags;
} Labeler;


#define SIZEOF_LABEL (22 + sizeof(TokenValue))

Label get_label_from_raw_data(const void* data);

void put_label_in_raw_data(const Label label, void* data);

const char* get_label_name(const void* label);

const char* get_label_def_as_str(const void* label);

Label* _get_label(const Mc_stream_t* labels, const Token label_tkn);

Label* get_label(const Labeler* labeler, const Token label_tkn);

int remove_label(Labeler* labeler, const Token label_token);

int add_label_with_flag(Labeler* labeler, const Token label_tkn, const Token definition, uint64_t inst_position, uint8_t flags);

int add_label(Labeler* labeler, const Token label_tkn, const Token definition, uint64_t inst_position);


Token resolve_token(const Labeler* labeler, const Token token);

int add_local_labelref(Labeler* labeler, int16_t* stride, const Token name, uint64_t refposition);

Label* get_missing_local_label(Mc_stream_t* local_labels);

int solve_local_label(Labeler* labeler, void* program, const Token name, uint64_t label_pos);

void destroy_labeler(Labeler labeler);

#endif
