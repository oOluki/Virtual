#ifndef VIRTUAL_LABLES_H
#define VIRTUAL_LABLES_H

#include "lexer.h"

enum LabelFlags{
    LABELFLAG_NONE = 0,
    LABELFLAG_EXPORT = 1 << 0,
    LABELFLAG_RESOLVED = 1 << 1,
};

typedef struct Label
{
    uint32_t   size;
    uint32_t   str;
    uint32_t   str_size;
    uint8_t    type;
    uint8_t    flags;
    TokenValue definition;
} Label;

typedef struct Labeler
{
    Mc_stream_t labels;
    Mc_stream_t local_labels;
} Labeler;


#define SIZEOF_LABEL (14 + sizeof(TokenValue))

static inline Label get_label_from_raw_data(const void* data){
    return (Label){
        .size               = ((uint32_t*) data)[0],
        .str                = ((uint32_t*) data)[1],
        .str_size           = ((uint32_t*) data)[2],
        .type               = ((uint8_t *) data)[3 * 4],
        .flags              = ((uint8_t *) data)[3 * 4 + 1],
        .definition.as_uint = *(uint64_t*)((uint8_t *) (data) + 3 * 4 + 2)
    };
}

static inline void put_label_in_raw_data(const Label label, void* data){
    uint32_t* const u32buffer = (uint32_t*) data;
    u32buffer[0] = label.size;
    u32buffer[1] = label.str;
    u32buffer[2] = label.str_size;
    *(uint8_t*)(u32buffer + 3) = label.type;
    *((uint8_t*)(u32buffer + 3) + 1) = label.flags;
    *(uint64_t*)((uint8_t*)(u32buffer + 3) + 2) = label.definition.as_uint;
}

static inline const char* get_label_name(const void* label){
    const Label l = get_label_from_raw_data(label);
    return (const char*)((uint8_t*)(label) + l.str);  
}

static inline const char* get_label_def_as_str(const void* label){
    const Label l = get_label_from_raw_data(label);
    return (const char*)((uint8_t*)(label) + l.definition.as_uint + sizeof(uint32_t));  
}

Label* get_label(const Mc_stream_t* labels, const Token label_tkn){
    for(size_t i = 0; i < labels->size; ){
        const uint8_t* data = (uint8_t*)(labels->data) + i;
        const Label label = get_label_from_raw_data(data);
        if(mc_compare_token(
                (Token){
                    .value.as_str = (char*)(data + label.str),
                    .size = label.str_size
                },
                label_tkn,
                0
            )
        ){
            return (Label*) data;
        }
        if(label.size == 0){
            fprintf(stderr, "[INTERNAL ERROR] The Labeler Got To An Invalid Label With Size 0. "
            "If You're An User Beware That This May Be A Problem With The Compiler Implementation And Not With Your Usage\n\n");
            return NULL;
        }
        i += label.size;
    }
    return NULL;
}

int remove_label(Mc_stream_t* labels, const Token label_token){
    Label* const label_ptr = get_label(labels, label_token);
    if(label_ptr == NULL) return 1;
    const Label label = get_label_from_raw_data(label_ptr);
    const uint32_t removed_label_size = label.size;
    const size_t ssize = (size_t)(labels->size - label.size - (size_t)((uint8_t*)(label_ptr) - (uint8_t*)(labels->data)));
    memmove(label_ptr, ((uint8_t*)label_ptr) + label.size, ssize);
    labels->size -= removed_label_size;
    return 0;
}

int add_label_with_flag(Mc_stream_t* labels, const Token label_tkn, const Token definition, uint8_t flags){

    if(get_label(labels, label_tkn)){
        fprintf(stderr, "[ERROR] Label Already Exists\n");
        return 1;
    }
    
    const int add_def_as_str = (definition.type == TKN_STR) || (definition.type == TKN_RAW);

    const Label label = (Label){
        .size = add_def_as_str? SIZEOF_LABEL + label_tkn.size + definition.size + sizeof(uint32_t) : SIZEOF_LABEL + label_tkn.size,
        .str = SIZEOF_LABEL,
        .str_size = label_tkn.size,
        .type = definition.type,
        .flags = flags,
        .definition.as_uint = add_def_as_str? SIZEOF_LABEL + label_tkn.size : definition.value.as_uint
    };

    const uint64_t ssize = labels->size;

    // reserve enough memory in labels stream to allocate label
    labels->size += label.size;
    mc_stream(labels, &labels, 0);

    void* const data = (uint8_t*)(labels->data) + ssize;

    put_label_in_raw_data(label, data);

    printf("label: '%.*s'\n", label_tkn.size, (char*) memcpy((uint8_t*)(data) + label.str, label_tkn.value.as_str, label_tkn.size));

    const uint32_t definition_size = (uint32_t) definition.size;
    // only copy definition.size and definition.value.as_str only if add_def_as_str
    memcpy((uint8_t*)(data) + label.definition.as_uint    , &definition_size, add_def_as_str? 4 : 0);
    memcpy((uint8_t*)(data) + label.definition.as_uint + 4, definition.value.as_str, add_def_as_str? definition.size : 0);

    return 0;
}

static inline int add_label(Mc_stream_t* labels, const Token label_tkn, const Token definition){
    return add_label_with_flag(labels, label_tkn, definition, LABELFLAG_NONE);
}


Token resolve_token(const Mc_stream_t* labels, const Token token){
    if(token.type != TKN_LABEL_REF) return token;
    const Label* const label_ptr = get_label(labels, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1});
    if(!label_ptr) return (Token){.value.as_str = token.value.as_str, .size = token.size, .type = TKN_ERROR};
    const Label label = get_label_from_raw_data(label_ptr);
    if(label.type == TKN_STR)
        return (Token){
            .value.as_str = (char*)((uint8_t*)(label_ptr) + label.definition.as_uint + sizeof(uint32_t)),
            .size = *(uint32_t*)((uint8_t*)(label_ptr) + label.definition.as_uint),
            .type = TKN_STR
        };
    return (Token){
        .value = label.definition,
        .size  = 0,
        .type  = label.type
    };
}

int add_local_labelref(Mc_stream_t* local_labels, int16_t* stride, const Token name, uint64_t refposition){
    if(name.type != TKN_RAW){
        fprintf(stderr, "[ERROR] Invalid Local Label Reference\n");
        return 1;
    }
    if(name.size < 1){
        fprintf(stderr, "[ERROR] Invalid Local Label Reference, Missing '@' Preffix\n");
        return 1;
    }
    if(name.value.as_str[0] != '@'){
        fprintf(stderr, "[ERROR] Invalid Local Label Reference, Missing '@' Preffix\n");
        return 1;
    }
    if(name.size < 2){
        fprintf(stderr, "[ERROR] Invalid Local Label Reference, Missing '.' After '@' Preffix\n");
        return 1;
    }
    if(name.value.as_str[1] != '.'){
        fprintf(stderr, "[ERROR] Invalid Local Label Reference, Missing '.' After '@' Preffix\n");
        return 1;
    }
    Label* lp = get_label(local_labels, (Token){.value.as_str = name.value.as_str + 1, .type = TKN_RAW, .size = name.size - 1});
    if(!lp){
        if(stride) *stride = 0;
        return add_label(
            local_labels,
            (Token){.value.as_str = name.value.as_str + 1, .type = TKN_RAW, .size = name.size - 1},
            (Token){.value.as_uint = refposition, .type = TKN_ULIT}
        );
    }
    Label label = get_label_from_raw_data(lp);

    const uint64_t last_pos = label.definition.as_uint;

    const int64_t o = (label.flags & LABELFLAG_RESOLVED)?
        (int64_t) (last_pos) - (int64_t)(refposition / sizeof(Inst))
                            :
        (int64_t) (last_pos) - (int64_t)(refposition);
    if(o > INT16_MAX || o < INT16_MIN){
        fprintf(stderr, "[ERROR] local label is too far from reference\n");
        return 1;
    }
    const int16_t output = (int16_t) o;
    if(stride) *stride = output;
    if(!(label.flags & LABELFLAG_RESOLVED)){
        label.definition.as_uint = refposition;
        put_label_in_raw_data(label, lp);
    }
    return 0;
}

Label* get_missing_local_label(Mc_stream_t* local_labels){
    if(!local_labels)
        return NULL;
    if(local_labels->size < SIZEOF_LABEL)
        return NULL;
    
    for(size_t i = 0; i < local_labels->size; ){
        const uint8_t* data = (uint8_t*)(local_labels->data) + i;
        const Label label = get_label_from_raw_data(data);
        if(!(label.flags & LABELFLAG_RESOLVED))
            return (Label*) data;
        if(label.size == 0){
            fprintf(stderr, "[INTERNAL ERROR] The Labeler Got To An Invalid Label With Size 0. "
            "If You're An User Beware That This May Be A Problem With The Compiler Implementation And Not With Your Usage\n\n");
            return NULL;
        }
        i += label.size;
    }
    return NULL;
}

int solve_local_label(Mc_stream_t* local_labels, void* program, const Token name, uint64_t label_pos){

    Label* lp = get_label(local_labels, name);
    if(!lp){
        return add_label_with_flag(local_labels, name, (Token){.value.as_uint = label_pos, .type = TKN_INST_POSITION, .size = 0}, LABELFLAG_RESOLVED);
    }

    Label label = get_label_from_raw_data(lp);

    if(label.flags & LABELFLAG_RESOLVED){
        fprintf(stderr, "[ERROR] Label Already Exists\n");
        return 1;
    }

    uint64_t pos = label.definition.as_uint;

    uint16_t stride = 0;

    for(memcpy(&stride, ((uint8_t*) program) + pos, sizeof(stride)); stride; memcpy(&stride, ((uint8_t*) program) + pos, sizeof(stride))){
        const int64_t o = (int64_t) (label_pos) - (int64_t)(pos / sizeof(Inst));
        if(o > INT16_MAX || o < INT16_MIN){
            fprintf(stderr, "[ERROR] local label is too far from reference\n");
            return 1;
        }
        const int16_t output = (int16_t) o;
        memcpy(((uint8_t*) program) + pos, &output, sizeof(output));
        pos += stride;
    }

    const int64_t o = (int64_t) (label_pos) - (int64_t)(pos / sizeof(Inst));
    if(o > INT16_MAX || o < INT16_MIN){
        fprintf(stderr, "[ERROR] local label is too far from reference\n");
        return 1;
    }
    const int16_t output = (int16_t) o;
    memcpy(((uint8_t*) program) + pos, &output, sizeof(output));

    label.flags |= LABELFLAG_RESOLVED;
    label.definition.as_uint = label_pos;
    put_label_in_raw_data(label, lp);

    return 0;
}

#endif
