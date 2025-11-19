#ifndef VIRTUAL_MEMC_HEADER
#define VIRTUAL_MEMC_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define VIRTUAL_DEBUG_MODE 1

#ifdef VIRTUAL_DEBUG_MODE

    #define DEBUG_CODE(X...) X

#else

    #define DEBUG_CODE(X...)

#endif // END OF #ifdef VIRTUAL_DEBUG_MODE

#define VIRTUAL_DEBUG_LOG(X...) DEBUG_CODE(do { fprintf(stderr, "[INFO] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, X); } while(0))
#define VIRTUAL_DEBUG_ERR(X...) DEBUG_CODE(do { fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, X); } while(0))
#define VIRTUAL_DEBUG_WARN(X...) DEBUG_CODE(do { fprintf(stderr, "[WARNING] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, X); } while(0))

#define virtual_alloc(size) malloc(size)
#define virtual_free(ptr) free(ptr)

#define DEFER_ERROR(ERROR...) do { err = 1; fprintf(stderr, "[ERROR] " ERROR); goto defer; } while(0)


typedef struct Mc_stream_t{
    void*            data;
    uint64_t         size;
    uint64_t         capacity;
    uint8_t          alignment;
} Mc_stream_t;

static inline int is_little_endian(){ return (*(unsigned short *)"\x01\x00" == 0x01); }

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

#define is_char_numeric(CHARACTER) (get_digit(CHARACTER) >= 0)

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
int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null){
    unsigned int i = 0;
    for( ; str1[i] && (str1[i] == str2[i]); i+=1);
    return (str1[i] == str2[i]) || (_only_compare_till_first_null && (!str1[i] || !str2[i]));
}

static inline uint16_t mc_swap16(uint16_t x){
    return (
        ((x & 0X00FF) >> 8)  |
        ((x & 0XFF00) >> 24)
    );
}

static inline uint32_t mc_swap32(uint32_t x){
    return (
        ((x & 0X000000FF) << 24) |
        ((x & 0X0000FF00) << 8)  |
        ((x & 0X00FF0000) >> 8)  |
        ((x & 0XFF000000) >> 24)
    );
}

static inline uint64_t mc_swap64(uint64_t x){
    return (
        ((x & 0x00000000000000FFULL) << 56) |
        ((x & 0x000000000000FF00ULL) << 40) |
        ((x & 0x0000000000FF0000ULL) << 24) |
        ((x & 0x00000000FF000000ULL) << 8)  |
        ((x & 0x000000FF00000000ULL) >> 8)  |
        ((x & 0x0000FF0000000000ULL) >> 24) |
        ((x & 0x00FF000000000000ULL) >> 40) |
        ((x & 0xFF00000000000000ULL) >> 56)
    );
}

void* virtual_alloc_aligned(size_t size, uint8_t alignment){

    if(alignment == 0) alignment = 1;

    void* ptr = malloc(size + sizeof(void*) + alignment - 1);

    if(ptr == NULL){
        return NULL;
    }

    uintptr_t uptr = (uintptr_t) ptr;

    const uintptr_t ret_addr = uptr + sizeof(void*) + (alignment - (uptr % alignment)) % alignment;

    uptr = ret_addr - sizeof(void*);

    uptr -= uptr % sizeof(void*);

    *((void**) uptr) = ptr;

    return (void*) ret_addr;
}

int virtual_free_aligned(void* ptr){

    if(!ptr) return 1;

    uintptr_t uptr = (uintptr_t) ptr;

    uptr -= sizeof(void*);

    uptr -= uptr % sizeof(void*);

    free(* (void**) uptr);

    return 0;
}

// streams size bytes of data to stream
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream(Mc_stream_t* stream, const void* data, size_t size){
    if(size + stream->size > stream->capacity){
        if(stream->capacity == 0) stream->capacity = size + stream->size;
        else stream->capacity *= 1 + (size_t)((size + stream->size) / stream->capacity);
        void* old_data = stream->data;
        stream->data = virtual_alloc_aligned(stream->capacity, stream->alignment);
        memcpy(stream->data, old_data, stream->size);
        virtual_free_aligned(old_data);
    }
    void* const dest = (void*) (((uintptr_t) stream->data) + stream->size);
    if(data) memcpy(dest, data, size);
    stream->size += size;
    return dest;
}

// streams size bytes of data to stream in position properly aligned to alignment relative to the stream'start
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream_aligned(Mc_stream_t* stream, const void* data, size_t size, size_t alignment){

    if(alignment <= 1){
        return mc_stream(stream, data, size);
    }

    const size_t pad = (alignment - (stream->size % alignment)) % alignment;
    
    stream->size += pad;

    return mc_stream(stream, data, size);
}

// works like mc_stream but streams a null treminated string
void* mc_stream_str(Mc_stream_t* stream, const char* data){

    if(!data) return NULL;

    const size_t size = strlen(data);

    return mc_stream(stream, data, (size + 1) * sizeof(char));
}

static inline void* mc_stream_on(const Mc_stream_t* stream, uint64_t index){ return (void*)((uint8_t*)(stream->data) + index);}


Mc_stream_t mc_create_stream(uint64_t capacity, uint8_t alignment){
    return (Mc_stream_t){.data = virtual_alloc_aligned(capacity, alignment), .size = 0, .capacity = capacity, .alignment = alignment};
}

void mc_destroy_stream(Mc_stream_t stream){
    virtual_free_aligned(stream.data);
}


int mc_dump_memory(const void* src, size_t size, const char* output){
    DEBUG_CODE(
        if(!src){
            VIRTUAL_DEBUG_ERR("could not dump memory to '%s': stream == NULL\n", output);
            return 1;
        }
        FILE* f = fopen(output, "wb");
        if(!f){
            VIRTUAL_DEBUG_ERR("could not dump memory at %p to '%s': could fopen file\n", src, output);
            return 1;
        }
        const size_t written = fwrite(src, 1, size, f);
        if(written != size){
            VIRTUAL_DEBUG_ERR(
                "could not properlly dump memory at %p to '%s': expected to write %zu bytes, wrote %zu instead\n",
                src, output, size, written
            );
            fclose(f);
            return 1;
        }
        VIRTUAL_DEBUG_LOG("dumped %zu bytes from %p to '%s'\n", size, src, output);
        fclose(f);
    )
    return 0;
}

#endif // END OF BASIC ========================================================================