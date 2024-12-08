#ifndef MC_MEMORY_HEADER
#define MC_MEMORY_HEADER

// MyC Header Only Library 'memory.h' Available At https://github.com/oOluki/MyC

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/

#ifndef MC_SETUP_DONE   // setting up some MyC configurations ===============================
#define MC_SETUP_DONE 1



#ifdef _STDLIB_H // if the standard library is included:

typedef size_t Mc_size_t;

#endif


#if defined(__arm__) || defined(__aarch64__) || defined(MC_INITMACRO_FORCE_ARM) // ARM architectures


#define MC_ARCHITECTURE_ARM 1

#endif


#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || \
    defined(__ppc64__) || defined(MC_INITMACRO_FORCE_64BIT) // 64 bit architectures


    #include <stdint.h>

    #define MC_ARCHITECTURE_64BIT 1

    #ifndef _STDLIB_H // if the standard library is not included:

        typedef uint64_t Mc_size_t;

    #endif

    typedef uint64_t Mc_uint_t;

    typedef int64_t Mc_int_t;

    #define MC_FLOAT double



#elif defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(MC_INITMACRO_FORCE_32BIT) // 32 bit architectures


    #include <stdint.h>

    #define MC_ARCHITECTURE_32BIT 1

    #ifndef _STDLIB_H // if the standard library is not included:

        typedef uint32_t Mc_size_t;

    #endif

    typedef uint32_t Mc_uint_t;

    typedef int32_t Mc_int_t;

#define MC_FLOAT float


#else // [WARNING] Unknown Architecture, this architecture could be unsupported and, if it doesn't support <stdint.h>, for example, it will lead to undefined behavior

    #ifndef _STDLIB_H

        typedef unsigned int Mc_size_t;

    #endif

    typedef unsigned int Mc_uint_t;

    typedef int Mc_int_t;


#endif // END OF ARCHITECTURE DEFINITION

#endif // END OF SETUP =====================================================

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*--------------------------------------------------/
/                                                   /
/  Simple Convinent String Methods Implementations  /
/                                                   /
/--------------------------------------------------*/

typedef struct Mc_string_t
{
    // not necessarily null terminated
    char* c_str;
    Mc_size_t size;
} Mc_string_t;

// if you only wish to compare the strings up to where the first one terminates, pass _only_compare_till_first_null=1
// otherwise pass _only_compare_till_first_null=0
static inline int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null){
    if(_only_compare_till_first_null){
        for(Mc_uint_t i = 0; str1[i] && str2[i]; i+=1){
            if(str1[i] != str2[i]) return 0;
        }
        return 1;
    }
    
    Mc_uint_t i = 0;
    for( ; str1[i]; i+=1){
        if(str1[i] != str2[i]) return 0;
    }
    return !str2[i];
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline Mc_int_t mc_find_char(const char* str, char c, Mc_int_t offset){
    str += offset;
    for(Mc_int_t i = 0; str[i]; i+=1){
        if(c == str[i]) return i;
    }
    return -1;
}

// works similar to cym_find_char but does so to any of the characters in the charbuff str
static inline Mc_int_t mc_find_chars(const char* str, const char* charbuff, Mc_int_t offset){
    str += offset;
    for(Mc_int_t i = 0; str[i]; i+=1){
        for(Mc_int_t j = 0; charbuff[j]; j+=1){
            if(charbuff[j] == str[i]) return i;
        }
    }
    
    return -1;
}


static inline int mc_scompare_str(const Mc_string_t str1, const Mc_string_t str2){
    if(str1.size != str2.size){
        return 0;
    }
    for(Mc_size_t i = 0; i < str1.size; i+=1){
        if(str1.c_str[i] != str2.c_str[i]){
            return 0;
        }
    }
    return 1;
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline Mc_int_t mc_sfind_char(Mc_string_t str, char c, Mc_int_t offset){
    str.c_str += offset;
    for(Mc_int_t i = 0; i < str.size; i+=1){
        if(c == str.c_str[i]) return i;
    }
    return -1;
}

// works similar to cym_sfind_char but does so to any of the characters in the charbuff str
static inline Mc_int_t mc_sfind_chars(Mc_string_t str, Mc_string_t charbuffstr, Mc_int_t offset){
    str.c_str += offset;
    for(Mc_int_t i = 0; i < str.size; i+=1){
        for(Mc_int_t j = 0; j < charbuffstr.size; j+=1){
            if(charbuffstr.c_str[j] == str.c_str[i]) return i;
        }
    }
    return -1;
}

/*--------------------------------/
/                                 /
/  Memory Related Implementations /
/                                 /
/--------------------------------*/

typedef char Mc_byte_t;


typedef struct Mc_stream_t{
    // data buffer
    Mc_byte_t*  data;
    // size of the stream in bytes
    uint64_t   size;
    // capacity of the stream in bytes
    uint64_t   capacity;
} Mc_stream_t;




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



#define MC_GET_ELEMENT_FROM_STREAM(STREAM, TYPE, POS) (*(TYPE*)(STREAM.data + POS))
#define MC_PUSH_ELEMENT_TO_STREAM(STREAM, TYPE, ELEMENT) do{\
    if(sizeof(TYPE) + STREAM->size > STREAM->capacity){\
        Mc_byte_t* old_data = STREAM->data;\
        STREAM->capacity *= 1 + (Mc_size_t)((size + STREAM->size) / STREAM->capacity);\
        STREAM->data = (Mc_byte_t*)malloc(STREAM->capacity);\
        memcpy(STREAM->data, old_data, STREAM->size);\
        free(old_data);\
    }\
    *(TYPE*)(STREAM->data + STREAM->size) = ELEMENT;\
    STREAM->size += sizeof(TYPE)\
    }while (0);
    

void mc_put(Mc_stream_t* stream, char c){
    if(1 + stream->size > stream->capacity){
        Mc_byte_t* old_data = stream->data;
        stream->capacity *= 2;
        stream->data = (Mc_byte_t*)malloc(stream->capacity);
        memcpy(stream->data, old_data, stream->size);
        free(old_data);
    }
    stream->data[stream->size] = c;
    stream->size += 1;
}

// streams size bytes of data to stream
void mc_stream(Mc_stream_t* stream, const void* data, Mc_size_t size){
    if(size + stream->size > stream->capacity){
        Mc_byte_t* old_data = stream->data;
        stream->capacity *= 1 + (Mc_size_t)((size + stream->size) / stream->capacity);
        stream->data = (Mc_byte_t*)malloc(stream->capacity);
        memcpy(stream->data, old_data, stream->size);
        free(old_data);
    }
    memcpy(stream->data + stream->size, data, size);
    stream->size += size;
}

// works like mc_stream but streams a null treminated string
void mc_stream_str(Mc_stream_t* stream, const char* data){
    Mc_size_t size = 0;

    while (data[size++]);

    mc_stream(stream, data, size * sizeof(char));
}

// reads size bytes from stream to output
void mc_read_stream(void* output, const Mc_stream_t stream, Mc_size_t offset, Mc_size_t size){

    memcpy(output, stream.data + offset, size);

}

// \returns (Mc_stream_t){.data = (Mc_byte_t*)malloc(capacity), .size = 0, .capacity = capacity};
Mc_stream_t mc_create_stream(Mc_size_t capacity){
    return (Mc_stream_t){.data = (Mc_byte_t*)malloc(capacity), .size = 0, .capacity = capacity};
}

// this simply passes stream.data to free. free(stream.data)
void mc_destroy_stream(Mc_stream_t stream){
    free(stream.data);
}

// changes the capacity of the stream to new_cap, resizing it accordingly
void mc_change_stream_cap(Mc_stream_t* stream, Mc_size_t new_cap){
    stream->capacity = new_cap;

    Mc_byte_t* old_data = stream->data;

    stream->data = (Mc_byte_t*)malloc(new_cap);

    if(new_cap < stream->size) stream->size = new_cap;

    memcpy(stream->data, old_data, stream->size);

    free(old_data);
}

/*-----------------------------------------------------------------/
/                                                                  /
/  This Is An Implementation Of A Tokenizer Or Lexer If You Prefer /
/                                                                  /
/-----------------------------------------------------------------*/

// tokens are just numbers, use them to get the corresponding token data through mc_get_token_data
typedef Mc_size_t Mc_token_t;

typedef struct Mc_tkn_metadata_t{

Mc_stream_t streamstring;
Mc_stream_t tokn_strm_bff;

} Mc_tkn_metadata_t;


static inline void add_token(Mc_stream_t* stream, Mc_stream_t* tokens, const char* str, int size){

    if(size < 0){
        const Mc_token_t token = stream->size;
        mc_stream_str(stream, str);
        mc_stream(tokens, &token, sizeof(token));
        return;
    }

    const Mc_token_t token = stream->size;
    mc_stream(stream, str, size);
    const char null_termc = '\0';
    mc_stream(stream, &null_termc, sizeof(null_termc));
    mc_stream(tokens, &token, sizeof(token));
}


// returns the number of generated tokens and writes the tokens key to 'token_buffer_pointer'
// the memory at the returned pointer has to be later freed via mc_destroy_tokens (do not use free)
// \param token_buffer_pointer a pointer to the buffer where the token keys will be stored
// \param buff_pos the position where to start buffering the tokens into the token_buffer
// \param string the string of to be tokenized
// \param __ignored the string of the characters to be ignored, pass null to ignore trivials '\ ' '\\n' '\\t'
// \param __special_characters the characters that should be considered tokens on their own
// \param __line_comment the string that represents a line comment
// \returns the number of tokenized tokens
size_t mc_tokenize(Mc_token_t** token_buffer_pointer, size_t buff_pos, const char* string,
    const char* __ignored, const char* __special_characters, const char* __line_comment, char str_sep){

        Mc_token_t* token_buffer = *token_buffer_pointer;
        
        Mc_tkn_metadata_t* meta = (Mc_tkn_metadata_t*)(token_buffer) - 1;
        Mc_stream_t stream = meta->streamstring;
        Mc_stream_t tokens = meta->tokn_strm_bff;

        if(buff_pos * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t) < tokens.size){
            stream.size = token_buffer[buff_pos];
            tokens.size = buff_pos + sizeof(Mc_tkn_metadata_t);
        }

        const char* ignored = __ignored? __ignored : " \t\n";               // avoiding null dereferencing
        const char* special_characters = __special_characters? __special_characters : "";
        const char* line_comment = __line_comment? __line_comment : "";

        Mc_size_t last_token = 0;
        Mc_size_t counter = 0;
        int last_was_ignored = 0;
        unsigned int tkn_count = 0;

        for(; string[counter]; counter+=1){
            const char c = string[counter];

            if(c == str_sep){
                if(last_token < counter){
                    add_token(&stream, &tokens, string + last_token, counter - last_token);
                    tkn_count += 1;
                }

                char dummybuff[3] = {str_sep, '\n', '\0'};

                int64_t skip = mc_find_chars(string, dummybuff, counter + 1);

                if(skip < 0) break;
                if(string[counter + 1 + skip] == '\n') break;
                add_token(&stream, &tokens, string + counter, skip + 2);
                counter += skip + 1;
                last_token = counter + 1;
                tkn_count += 1;

            }

            if(c == line_comment[0]){
                int skip = mc_compare_str(string + counter, line_comment, 1);
                
                if(skip){
                    if(!last_was_ignored && (last_token < counter)){
                        add_token(&stream, &tokens, string + last_token, counter - last_token);
                        tkn_count += 1;
                    }
                    Mc_int_t to_ = mc_find_char(string, '\n', counter);
                    if(to_ <= 0) break;
                    counter += to_ - 1;
                    last_token = counter + 1;
                    continue;
                }
            }

            Mc_byte_t is_ignored = (mc_find_char(ignored, c, 0) >= 0)? 1 : 0;
            if(is_ignored){
                if(!last_was_ignored && (last_token < counter)){
                    add_token(&stream, &tokens, string + last_token, counter - last_token);
                    tkn_count += 1;
                }
                last_token = counter + 1;
                continue;
            } else last_was_ignored = 0;

            Mc_byte_t is_special = (mc_find_char(special_characters, c, 0) >= 0)? 1 : 0;

            if(is_special){
                if(last_token < counter){
                    add_token(&stream, &tokens, string + last_token, counter - last_token);
                    tkn_count += 1;
                }
                add_token(&stream, &tokens, string + counter, 1);
                last_token = counter + 1;
                tkn_count += 1;
            }

        }
        if(last_token < counter){
            add_token(&stream, &tokens, string + last_token, counter - last_token);
            tkn_count += 1;
        }
        

        *token_buffer_pointer = (Mc_token_t*)((Mc_tkn_metadata_t*)tokens.data + 1);

        meta = (Mc_tkn_metadata_t*)tokens.data;
        *meta = (Mc_tkn_metadata_t){.streamstring = stream, .tokn_strm_bff = tokens};


        return tkn_count;
}



// you can repeatedly use this bufffer on mc_tokenize only by the end
// should this be freed by mc_destroy_token_buffer (don't use free)
// \param tkn_icap the initial capacity (in token count) of the token buffer
// \param tknstrm_icap the initial capacity (in bytes) of the stream that will allocate the tokens strings
// \returns a buffer that can be used to hold the tokenized tokens by mc_tokenize
Mc_token_t* mc_create_token_buffer(Mc_size_t tkn_icap, Mc_size_t tknstrm_icap){

    Mc_tkn_metadata_t* mem_chunk = (Mc_tkn_metadata_t*)malloc(
        tkn_icap * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t)
    );

    mem_chunk->tokn_strm_bff.data = (Mc_byte_t*)(mem_chunk);
    mem_chunk->tokn_strm_bff.size = sizeof(Mc_tkn_metadata_t);
    mem_chunk->tokn_strm_bff.capacity = tkn_icap * sizeof(Mc_token_t) + sizeof(Mc_tkn_metadata_t);

    mem_chunk->streamstring.data = (Mc_byte_t*)malloc(tknstrm_icap);
    mem_chunk->streamstring.size = 0;
    mem_chunk->streamstring.capacity = tknstrm_icap;

    return (Mc_token_t*)(mem_chunk + 1);
}

// frees all the memory related to the tokenization process of tokens
// (their strings and the memory chunk containing them and the tokens)
// \param the pointer to the tokens to destroy
void mc_destroy_token_buffer(Mc_token_t* tokens){
    Mc_tkn_metadata_t* mem_chunk = (Mc_tkn_metadata_t*)tokens - 1;
    free(mem_chunk->streamstring.data);
    free(mem_chunk);
}


// returns the string related to the token in the token_buffer
static inline char* mc_get_token_data(const Mc_token_t* token_buffer, Mc_token_t token){
    return ((const Mc_tkn_metadata_t*)token_buffer - 1)->streamstring.data + token;
}

// buffers the strings related to all the tokens in the token_buffer to the corresponding string in buffer
// this is simply the meta_data.str_begin + token
void mc_buffer_token_data(const char** buffer, const Mc_token_t* token_buffer){
    const Mc_tkn_metadata_t* meta_data = (Mc_tkn_metadata_t*)token_buffer - 1;
    const Mc_size_t token_count = (meta_data->tokn_strm_bff.size - sizeof(Mc_tkn_metadata_t)) / sizeof(Mc_token_t);
    for(Mc_size_t i = 0; i < token_count; i+=1){
        buffer[i] = meta_data->streamstring.data + token_buffer[i];        
    }
}


void print_tokens(Mc_token_t* token_buffer, size_t token_count){

    for(size_t i = 0; i < token_count; i++){
        printf(" '%s' ", mc_get_token_data(token_buffer, token_buffer[i]));
    }
    printf("\n");
}

char* read_file(Mc_stream_t* stream, const char* path, const char* modes){

    FILE* file = fopen(path, modes);
    if(!file) return NULL;

    if(fseek(file, 0, SEEK_END)){
        fclose(file);
        return NULL;
    }

    long size = ftell(file);

    if(fseek(file, 0, 0)){
        fclose(file);
        return NULL;
    }

    if(size < 0){
        fclose(file);
        return NULL;
    }

    const size_t issize = stream->size;

    if(stream->size + size > stream->capacity){
        stream->capacity = (size_t)(size + stream->size);
        char* odata = stream->data;
        stream->data = (char*)malloc(stream->capacity);
        memmove(stream->data, odata, stream->size);
        free(odata);
    }

    stream->size += fread(stream->data + stream->size, 1, size, file);

    fclose(file);

    return stream->data + issize;
}



#endif // END OF FILE

