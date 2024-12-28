#ifndef VIRTUAL_LABLES_H
#define VIRTUAL_LABELS_H

#include "lexer.h"

typedef struct Label
{
    uint32_t size;
    uint32_t str;
    uint32_t str_size;
    Token definition;
} Label;


Label* get_label(Mc_stream_t* labels, const Token label_tkn){
    for(size_t i = 0; i < labels->size; ){
        //const uint32_t label_pos = *(uint32_t*)((uint8_t*)(labels->data) + i);
        Label* const label = (Label*)((uint8_t*)(labels->data) + i);
        if(mc_compare_token(
                (Token){(char*)(labels->data) + label->str, label->str_size, 0, 0},
                label_tkn,
                0
            )
        ){
            return label;
        }
        i += label->size;
    }
    return NULL;
}

int remove_label(Mc_stream_t* labels, const Token label_token){

    
    Label* label = get_label(labels, label_token);
    if(label == NULL) return 1;
    memcpy(label, ((uint8_t*)label) + label->size, label->size);

    return 0;
}

int add_label(Mc_stream_t* labels, const Token label_tkn, const Token definition, int hint){

    Label* label = get_label(labels, label_tkn);

    if(label) return 1;

    Label dummy_label;
    dummy_label.str = labels->size + sizeof(dummy_label);
    dummy_label.str_size = label_tkn.size;
    const size_t lpos = labels->size;
    mc_stream(labels, &dummy_label, sizeof(dummy_label));
    mc_stream(labels, label_tkn.value.as_str, label_tkn.size);
    label = (Label*)((uint8_t*)(labels->data) + lpos);

    if(definition.type == TKN_STR){
        label->definition.value.as_uint = labels->size;
        label->definition.size = definition.size;
        label->definition.type = TKN_STR;
        label->size = definition.size + labels->size - lpos;
        mc_stream(labels, definition.value.as_str, definition.size);
    }
    else{
        label->definition = definition;
        label->size = labels->size - lpos;
    }

    return 0;
}


Token resolve_token(Mc_stream_t* labels, const Token token){
    if(token.type != TKN_LABEL_REF) return token;
    const Label* const label = get_label(labels, (Token){.value.as_str = token.value.as_str + 1, .size = token.size - 1});
    if(!label) return (Token){.value.as_str = token.value.as_str, .size = token.size, .line = token.line, .column = token.column, .type = TKN_ERROR};
    return label->definition;
}

#endif