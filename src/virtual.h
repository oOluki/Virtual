#ifndef VIRTUAL_MEMC_HEADER
#define VIRTUAL_MEMC_HEADER

#include "core.h"
#include <stdint.h>

//#define VIRTUAL_DEBUG_MODE 1

#ifdef VIRTUAL_DEBUG_MODE

    #define DEBUG_CODE(...) __VA_ARGS__

#else

    #define DEBUG_CODE(...)

#endif // END OF #ifdef VIRTUAL_DEBUG_MODE

#define VIRTUAL_DEBUG_LOG(...) DEBUG_CODE(do { fprintf(stderr, "[INFO] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, __VA_ARGS__); } while(0))
#define VIRTUAL_DEBUG_ERR(...) DEBUG_CODE(do { fprintf(stderr, "[ERROR] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, __VA_ARGS__); } while(0))
#define VIRTUAL_DEBUG_WARN(...) DEBUG_CODE(do { fprintf(stderr, "[WARNING] " __FILE__ ":%i:0: ", __LINE__); fprintf(stderr, __VA_ARGS__); } while(0))

#define virtual_alloc(size) malloc(size)
#define virtual_free(ptr) free(ptr)

#define DEFER_ERROR(...) do { err = 1; fprintf(stderr, "[ERROR] " __VA_ARGS__); goto defer; } while(0)

#ifndef VPU_MEMALIGN_TO
    #define VPU_MEMALIGN_TO 8
#endif

#ifndef VERSION
    #define VERSION "VERSION NOT PROVIDED"
#endif

typedef struct Mc_stream_t{
    void*            data;
    uint64_t         size;
    uint64_t         capacity;
    uint8_t          alignment;
} Mc_stream_t;

static inline int is_little_endian(){ return (*(unsigned short *)"\x01\x00" == 0x01); }

int get_digit(char c);

#define is_char_numeric(CHARACTER) (get_digit(CHARACTER) >= 0)

char get_char_digit(int d);

uint8_t get_hex_digit(char c);

// if you only wish to compare the strings up to where the first one terminates, pass _only_compare_till_first_null=1
// otherwise pass _only_compare_till_first_null=0
int mc_compare_str(const char* str1, const char* str2, int _only_compare_till_first_null);

uint16_t mc_swap16(uint16_t x);

uint32_t mc_swap32(uint32_t x);

uint64_t mc_swap64(uint64_t x);

void* virtual_alloc_aligned(size_t size, uint8_t alignment);

int virtual_free_aligned(void* ptr);

// streams size bytes of data to stream
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream(Mc_stream_t* stream, const void* data, size_t size);

// streams size bytes of data to stream in position properly aligned to alignment relative to the stream'start
// \param data the data to stream, pass NULL to allocate the memory but not stream it
// \returns pointer to beggining of streamed data in stream
void* mc_stream_aligned(Mc_stream_t* stream, const void* data, size_t size, size_t alignment);

// works like mc_stream but streams a null treminated string
void* mc_stream_str(Mc_stream_t* stream, const char* data);

static inline void* mc_stream_on(const Mc_stream_t* stream, uint64_t index){ return (void*)((uint8_t*)(stream->data) + index);}


Mc_stream_t mc_create_stream(uint64_t capacity, uint8_t alignment);

void mc_destroy_stream(Mc_stream_t stream);

DEBUG_CODE(
    int mc_dump_memory(const void* src, size_t size, const char* output);
)

#endif // END OF BASIC ========================================================================