#ifndef VIRTUAL_LABLES_H
#define VIRTUAL_LABLES_H

#include "lexer.h"

typedef struct Label
{
    uint32_t size;
    uint32_t str;
    uint32_t str_size;
    Token definition;
} Label;


Label* get_label(const Mc_stream_t* labels, const Token label_tkn){
    for(size_t i = 0; i < labels->size; ){
        //const uint32_t label_pos = *(uint32_t*)((uint8_t*)(labels->data) + i);
        Label* const label = (Label*)((uint8_t*)(labels->data) + i);
        if(mc_compare_token(
                (Token){
                    .value.as_str = (char*)((uint8_t*)(label) + label->str),
                    .size = label->str_size
                },
                label_tkn,
                0
            )
        ){
            return label;
        }
        if(label->size == 0){
            fprintf(stderr, "[INTERNAL ERROR] The Labeler Got To An Invalid Label With Size 0. "
            "If You're An User Beware That This May Be A Problem With The Compiler Implementation And Not With Your Usage\n\n");
            return NULL;
        }
        //printf("%u - %u - %u\n", (unsigned int)i, (unsigned int)labels->size, (unsigned int)label->size);
        i += label->size;
    }
    return NULL;
}

int remove_label(Mc_stream_t* labels, const Token label_token){
    Label* label = get_label(labels, label_token);
    if(label == NULL) return 1;
    const uint32_t removed_label_size = label->size;
    const size_t ssize = (size_t)(labels->size - label->size - (size_t)((uint8_t*)(label) - (uint8_t*)(labels->data)));
    memmove(label, ((uint8_t*)label) + label->size, ssize);
    labels->size -= removed_label_size;
    return 0;
}

int add_label(Mc_stream_t* labels, const Token label_tkn, const Token definition){

    const Label* dummy_label = get_label(labels, label_tkn);

    if(dummy_label) return 1;

    Label label;
    label.str = sizeof(label);
    label.str_size = label_tkn.size;

    if(definition.type == TKN_STR){
        label.definition.value.as_uint = sizeof(label) + label.str_size;
        label.definition.size = definition.size;
        label.definition.type = TKN_STR;
        label.size = sizeof(label) + label_tkn.size + definition.size;
        mc_stream(labels, &label, sizeof(label));
        mc_stream(labels, label_tkn.value.as_str, label_tkn.size);
        mc_stream(labels, definition.value.as_str, definition.size);
        return 0;
    }

    label.definition = definition;
    label.size = sizeof(label) + label_tkn.size;
    mc_stream(labels, &label, sizeof(label));
    mc_stream(labels, label_tkn.value.as_str, label_tkn.size);

    return 0;
}


Token resolve_token(const Mc_stream_t* labels, const Token token){
    if(token.type != TKN_LABEL_REF) return token;
    const Label* const label = get_label(labels, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1});
    if(!label) return (Token){.value.as_str = token.value.as_str, .size = token.size, .type = TKN_ERROR};
    if(label->definition.type == TKN_STR)
        return (Token){
            .value.as_str = (char*)((uint8_t*)(label) + label->definition.value.as_uint),
            .size = label->definition.size,
            .type = TKN_STR
        };
    return label->definition;
}

#endif
