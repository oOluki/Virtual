#ifndef VIRTUAL_LEXER_H
#define VIRTUAL_LEXER_H


#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "core.h"


typedef struct Mc_stream_t{
    // data buffer
    void*  data;
    // size of the stream in bytes
    uint64_t   size;
    // capacity of the stream in bytes
    uint64_t   capacity;
} Mc_stream_t;

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
    TKN_SPECIAL_SYM,
    TKN_MACRO_INST,
    TKN_LABEL_REF,
    TKN_EMPTY,
    TKN_ADDR_LABEL_REF,
    TKN_STATIC_SIZE,
    TKN_ERROR = 255,
};

typedef struct StringView
{
    char*    str;
    uint32_t size;
} StringView;


#define MKTKN(STR) ((Token){.value.as_str = STR, .size = sizeof(STR) - 1, .type = TKN_RAW})

#define is_char_numeric(CHARACTER) (get_digit(CHARACTER) >= 0)


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

// if you only wish to compare the strings up to where the first one terminates, pass _only_compare_till_first_null=1
// otherwise pass _only_compare_till_first_null=0
static inline int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null){
    if(_only_compare_till_first_null){
        for(unsigned int i = 0; str1[i] && str2[i]; i+=1){
            if(str1[i] != str2[i]) return 0;
        }
        return 1;
    }
    
    unsigned int i = 0;
    for( ; str1[i]; i+=1){
        if(str1[i] != str2[i]) return 0;
    }
    return !str2[i];
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline int mc_find_char(const char* str, char c, int offset){
    str += offset;
    for(int i = 0; str[i]; i+=1){
        if(c == str[i]) return i;
    }
    return -1;
}

// works similar to cym_find_char but does so to any of the characters in the charbuff str
static inline int mc_find_chars(const char* str, const char* charbuff, int offset){
    str += offset;
    for(int i = 0; str[i]; i+=1){
        for(int j = 0; charbuff[j]; j+=1){
            if(charbuff[j] == str[i]) return i;
        }
    }
    
    return -1;
}





static inline uint32_t mc_swap32(uint32_t x){
    return (
        ((x & 0X000000FF) << 24) |
        ((x & 0X0000FF00) << 8) |
        ((x & 0X00FF0000) >> 8) |
        ((x & 0XFF000000) >> 24)
    );
}



static inline uint64_t mc_swap64(uint64_t x) {
    return ((x & 0x00000000000000FFULL) << 56) |
        ((x & 0x000000000000FF00ULL) << 40) |
        ((x & 0x0000000000FF0000ULL) << 24) |
        ((x & 0x00000000FF000000ULL) << 8)  |
        ((x & 0x000000FF00000000ULL) >> 8)  |
        ((x & 0x0000FF0000000000ULL) >> 24) |
        ((x & 0x00FF000000000000ULL) >> 40) |
        ((x & 0xFF00000000000000ULL) >> 56);
}

// streams size bytes of data to stream
void mc_stream(Mc_stream_t* stream, const void* data, size_t size){
    if(size + stream->size > stream->capacity){
        void* old_data = stream->data;
        stream->capacity *= 1 + (size_t)((size + stream->size) / stream->capacity);
        stream->data = vpu_alloc_aligned(stream->capacity, 8);
        memcpy(stream->data, old_data, stream->size);
        vpu_free_aligned(old_data);
    }
    memcpy((uint8_t*)(stream->data) + stream->size, data, size);
    stream->size += size;
}

// works like mc_stream but streams a null treminated string
void mc_stream_str(Mc_stream_t* stream, const char* data){
    size_t size = 0;

    while (data[size++]);

    mc_stream(stream, data, size * sizeof(char));
}

// \returns (Mc_stream_t){.data = vpu_alloc_aligned(capacity, VPU_MEMALIGN_TO), .size = 0, .capacity = capacity};
Mc_stream_t mc_create_stream(uint64_t capacity){
    return (Mc_stream_t){.data = vpu_alloc_aligned(capacity, VPU_MEMALIGN_TO), .size = 0, .capacity = capacity};
}

// this simply passes stream.data to vpu_free_aligned. free(stream.data)
void mc_destroy_stream(Mc_stream_t stream){
    vpu_free_aligned(stream.data);
}


void tokenizer_goto(Tokenizer* tokenizer, const char* dest){

    for(; tokenizer->data[tokenizer->pos]; tokenizer->pos+=1){
        if(mc_compare_str(tokenizer->data + tokenizer->pos, dest, 1)){
            break;
        }
        if(tokenizer->data[tokenizer->pos] == '\n'){
            tokenizer->line += 1;
            tokenizer->column = 0;
            continue;
        }
        tokenizer->column += 1;
    }

}


Token get_next_token(Tokenizer* tokenizer){
    char* string = tokenizer->data;
    const char* special_characters = ":";
    const char line_comment = ';';
    const char* delimiters = " \t\n;:";

    Token token = (Token){0};
    
    for(; string[tokenizer->pos]; tokenizer->pos+=1){
        while(string[tokenizer->pos] == ' ' || string[tokenizer->pos] == '\t'){
            tokenizer->column += 1;
            tokenizer->pos += 1;
        }
        if(string[tokenizer->pos] == '\n'){
            tokenizer->line += 1;
            tokenizer->column = 0;
            continue;;
        }

        const char c = string[tokenizer->pos];
        if(c == '\"'){
            token.value.as_str = string + tokenizer->pos;
            token.type = TKN_STR;
            const int skip = mc_find_char(string + tokenizer->pos + 1, '\"', 0);
            if(skip < 0){
                for(token.size = 0; string[tokenizer->pos + token.size]; token.size += 1);
            }
            else token.size = skip + 2;
            tokenizer->pos += token.size;
            tokenizer->column += token.size;
            return token;
        }
        if(c == line_comment){
            int skip = mc_find_char(string + tokenizer->pos + 1, '\n', 0);
            if(skip < 0){
                return (Token){.value.as_str = NULL, .size = 0, .type = TKN_NONE};
            }
            tokenizer->pos += skip;
            tokenizer->column += skip + 1;
            continue;            
        }
        if(mc_find_char(special_characters, string[tokenizer->pos], 0) >= 0){
            token.value.as_str = (string + tokenizer->pos);
            token.size = 1;
            token.type = TKN_SPECIAL_SYM;
            tokenizer->pos += 1;
            tokenizer->column += 1;
            return token;
        }
        token.size = mc_find_chars(string + tokenizer->pos, delimiters, 0);
        if(token.size < 0 && string[tokenizer->pos]){
            for(token.size = 0; string[token.size + tokenizer->pos]; token.size+=1);
        } else if(token.size < 0){
            break;
        }
        token.value.as_str = string + tokenizer->pos;
        switch (token.value.as_str[0])
        {
        case '%':
            token.type = TKN_MACRO_INST;
            break;
        case '$':
            token.type = TKN_LABEL_REF;
            break;
	case '@':
	    token.type = TKN_ADDR_LABEL_REF;
	    break;
        
        default:
            token.type = TKN_RAW;
            break;
        }
        tokenizer->pos += token.size;
        tokenizer->column += token.size;
        return token;
    }
    token.type = TKN_NONE;
    return token;
}


static inline int mc_compare_token(const Token token1, const Token token2, int _only_compare_till_smaller){
    if(_only_compare_till_smaller == 0 && token1.size != token2.size)
        return 0;

    const unsigned int range = (token1.size < token2.size)? token1.size : token2.size;
    
    for(unsigned int i = 0; i < range; i+=1){
	if(token1.value.as_str[i] != token2.value.as_str[i]) return 0;
    }
    return 1;
}

// if include_file_path is NOT 0 then, on success, the file path will be streamed to the stream as streamview
// (first size (uint32) then cstr (null terminated)) before the file contents
char* read_file(Mc_stream_t* stream, const char* path, int binary, int include_file_path){

    FILE* file = fopen(path, binary? "rb" : "r");
    if(!file) return NULL;

    if(fseek(file, 0, SEEK_END)){
        fclose(file);
        return NULL;
    }

    const unsigned long size = (unsigned long) ftell(file);

    if(fseek(file, 0, 0)){
        fclose(file);
        return NULL;
    }

    if(size < 0){
        fclose(file);
        return NULL;
    }

    const uint64_t issize = stream->size;

    if(include_file_path){
        uint32_t path_str_size = 0;
        for ( ; path[path_str_size]; path_str_size+=1);
        mc_stream(stream, &path_str_size, sizeof(path_str_size));
        mc_stream(stream, path, path_str_size + 1);
    }

    if(stream->size + size + 1 > stream->capacity){
        stream->capacity = (size_t)(size + stream->size + 1);
        void* odata = stream->data;
        stream->data = (char*)vpu_alloc_aligned(stream->capacity, 8);
        memcpy(stream->data, odata, stream->size);
        vpu_free_aligned(odata);
    }

    stream->size += fread((uint8_t*)(stream->data) + stream->size, 1, size, file);

    if(!binary) ((char*)stream->data)[stream->size++] = '\0';

    fclose(file);

    return (char*)((uint8_t*)(stream->data) + issize);
}

// this automatically includes the concatonated file path as stringview (first size (uint32) then cstr (null terminated))
// to the stream before the file contents, only if on success
char* read_file_relative(Mc_stream_t* stream, const StringView mother_dir, const StringView relative_path){

    const uint64_t ssize = stream->size;
    const uint32_t path_str_size = mother_dir.size + relative_path.size;

    mc_stream(stream, &path_str_size, sizeof(path_str_size));
    mc_stream(stream, mother_dir.str, mother_dir.size);
    mc_stream(stream, relative_path.str, relative_path.size);
    const char nullterm_ = '\0';
    mc_stream(stream, &nullterm_, sizeof(nullterm_));

    char* const status = read_file(stream, (char*)((uint8_t*)(stream->data) + ssize + sizeof(path_str_size)), 0, 0);

    if(status == NULL){
        stream->size = ssize;
        return NULL;
    }

    return (char*)((uint8_t*)(stream->data) + ssize);
}

#endif
