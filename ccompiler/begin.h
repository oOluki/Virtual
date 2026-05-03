#ifndef BEGIN_H
#define BEGIN_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define report_error(FMT, ...) do{\
        fprintf(stderr, "[ERROR] %s:%i:%i: " FMT "\n", (tokenizer).src_file_name, (tokenizer).line, (tokenizer).column, __VA_ARGS__);\
        exit(EXIT_FAILURE);\
    } while(0)

#define report_internal_error(FMT, ...) do{ fprintf(stderr, "[INTERNAL ERROR] " __FILE__ ":%i:0: " FMT "\n", __LINE__, __VA_ARGS__); exit(EXIT_FAILURE); } while(0)

#define TODOF(FMT, ...) do{ fprintf(stderr, "[TODO] " __FILE__ ":%i:0: " FMT "\n", __LINE__, __VA_ARGS__); exit(EXIT_FAILURE); } while(0)
#define TODO(MSG) do{ fprintf(stderr, "[TODO] " __FILE__ ":%i:0: " MSG "\n", __LINE__); exit(EXIT_FAILURE); } while(0)


#define ARLEN(X) (sizeof(X) / sizeof((X)[0]))

#define da_append(da, X, TYPE) do{\
        if((da)->size + sizeof(TYPE) >= (da)->capacity){\
            void* old = (da)->data;\
            if((da)->capacity == 0) (da)->capacity = sizeof(TYPE) + (da)->size;\
            else (da)->capacity *= 1 + (size_t)((sizeof(TYPE) + (da)->size) / (da)->capacity);\
            (da)->data = malloc((da)->capacity);\
            memcpy((da)->data, old, (da)->size);\
            free(old);\
        }\
        *(TYPE*) (((uint8_t*) (da)->data) + (da)->size) = (X);\
        (da)->size += sizeof(TYPE);\
    } while (0);

#define da_for_each(da, X, TYPE, BLOCK) do{\
        for(TYPE* X = (TYPE* const) (da)->data; X < ((uint8_t*) (da)->data) + (da)->size; X+=1){\
            BLOCK;\
        }\
    } while (0);

#define da_pop(da, X, TYPE) do{\
        X = *(TYPE*) da_back(da, sizeof(TYPE));\
    } while (0);

#define da_element(da, index, TYPE) (((TYPE*) (da)->data) + index)

#define da_len(da, TYPE) ((da)->size / sizeof(TYPE))

enum Types {
    TYPE_VOID = 0,
    TYPE_CHAR,
    TYPE_UCHAR,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_PTR,

    TYPE_VAR,
    TYPE_FUNC,

    TYPE_ERROR,
};

typedef struct Str
{
    const char* cstr;
    int         size;
} Str;

typedef struct DynamicArray
{
    void*  data;
    size_t size;
    size_t capacity;
} DyArr;


typedef union TokenValue{
    char            c;
    unsigned char   uc;
    int             i;
    unsigned int    u;
    float           f;
    double          lf;
    void*           p;
    char*           cstr;
    Str             str;
} TokenValue;


Str read_file(const char* file);

int compare_str(Str str1, Str str2);

int comp_str_cstr(Str str, const char* cstr);

void* da_stream(DyArr* da, const void* X, size_t SIZE);

void* da_back(DyArr* da, size_t size);

#endif // =====================  END OF FILE BEGIN_H ===========================