#ifndef VSYSTEM_C
#define VSYSTEM_C

#ifdef SDL_SYSTEM

    #include "sdl2_system.c"

#elif defined(CUSTOM_SYSTEM)

#else

#include "core.h"
#include "system.h"

typedef struct VSystem
{
    uint32_t* display;
    uint32_t  display_width;
    uint32_t  display_height;

} VSystem;


static char get_ascii_color(uint32_t color){

    static const char ascii_map[] = "@#Xx%*+=~-:\"'. ";
    const  int ascii_maplen = sizeof(ascii_map) / sizeof(ascii_map[0]) - 1;

    const uint32_t rw = 2126;
    const uint32_t gw = 7152;
    const uint32_t bw =  722;

    const uint32_t r = ((color >>  0) & 0xFF);
    const uint32_t g = ((color >>  8) & 0xFF);
    const uint32_t b = ((color >> 16) & 0xFF);
    const uint32_t a = ((color >> 24) & 0xFF);

    const uint32_t brightness = a? (rw * r + gw * g + bw * b) / (rw + gw + bw) : 0;
    const int ascii_index = (brightness < 255)? (brightness * ascii_maplen) / 255 : ascii_maplen - 1;
    return ascii_map[ascii_maplen - 1 - ascii_index];
}


int virtual_syscall(VPU* vpu, uint64_t call){
    if(!vpu->system && !(call == VSYS_GET_SYSTEM_SPECIFICATIONS || call == VSYS_INITIALIZE))
        return 1;

    void* const registers = vpu->register_space;

    switch (call)
    {
    case VSYS_GET_SYSTEM_SPECIFICATIONS:
        // system name goes in RA
        GET_REG(registers, RA)->as_uint64   = *(uint64_t*) "DEFAULT";
        // system bitsize architecture goes in RB
        GET_REG(registers, RB)->as_uint8    = sizeof(size_t);
        // get system active state to RC
        GET_REG(registers, RC)->as_uint64   = (vpu->system != NULL);
        // system endianess goes in RD
        GET_REG(registers, RD)->as_uint8    = is_little_endian()? 1 : 0;
        return 0;
    case VSYS_INITIALIZE:{
        VSystem* const system = virtual_alloc(sizeof(VSystem));
        if(!system)
            return 1;

        memset(system, 0, sizeof(*system));

        system->display = virtual_alloc(GET_REG(registers, RA)->as_uint32 * GET_REG(registers, RA4)->as_uint32);
        if(!system->display){
            virtual_free(system);
            return 1;
        }
        system->display_width  = GET_REG(registers, RA)->as_uint32;
        system->display_height = GET_REG(registers, RA4)->as_uint32;
        vpu->system = system;
    }
        return 0;
    case VSYS_CLOSE:
        virtual_free(vpu->system);
        vpu->system = NULL;
        return 0;
    
    // heap
    case VSYS_MEMALLOC:
        GET_REG(registers, RA)->as_ptr = virtual_alloc((size_t) GET_REG(registers, RA)->as_uint64);
        return 0;
    case VSYS_MEMFREE:
        virtual_free(GET_REG(registers, RA)->as_ptr);
        return 0;
    
    // file operations
    case VSYS_FOPEN:
        GET_REG(registers, RA)->as_ptr = (uint8_t*) fopen(
            (const char*) GET_REG(registers, RA)->as_ptr,
            (const char*) GET_REG(registers, RB)->as_ptr
        );
        return 0;
    case VSYS_FCLOSE:
        GET_REG(registers, RA)->as_int32 = fclose((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case VSYS_FSEEK:
        GET_REG(registers, RA)->as_int32 = fseek(
            (FILE*) GET_REG(registers, RA)->as_ptr,
            (long)  GET_REG(registers, RB)->as_int64,
            (int)   GET_REG(registers, RC)->as_int32
        );
        return 0;
    case VSYS_FTELL:
        GET_REG(registers, RA)->as_int64 = ftell((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case VSYS_FWRITE:
        GET_REG(registers, RA)->as_uint64 = (uint64_t) fwrite(
            (void*)  GET_REG(registers, RA)->as_ptr,
            (size_t) GET_REG(registers, RB)->as_uint64,
            (size_t) GET_REG(registers, RC)->as_uint64,
            (FILE*)  GET_REG(registers, RD)->as_ptr
        );
        return 0;
    case VSYS_FREAD:
        GET_REG(registers, RA)->as_uint64 = (uint64_t) fread(
            (void*)  GET_REG(registers, RA)->as_ptr,
            (size_t) GET_REG(registers, RB)->as_uint64,
            (size_t) GET_REG(registers, RC)->as_uint64,
            (FILE*)  GET_REG(registers, RD)->as_ptr
        );
        return 0;
    case VSYS_FPUTC:
        GET_REG(registers, RA)->as_int64 = fputc(GET_REG(registers, RA)->as_int8, (FILE*) GET_REG(registers, RB)->as_ptr);
        return 0;
    case VSYS_FGETC:
        GET_REG(registers, RA)->as_int64 = fgetc((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case VSYS_IOE:
        GET_REG(registers, RA)->as_ptr = stdin;
        GET_REG(registers, RB)->as_ptr = stdout;
        GET_REG(registers, RC)->as_ptr = stderr;
        return 0;
    
    // no threads...

    case VSYS_NEW_THREAD:
    case VSYS_WAIT_THREAD:
    case VSYS_DETACH_THREAD:
    case VSYS_SLEEP:
        return 0;

    case VSYS_GET_DISPLAY_FRAMEBUFFER:{
        const VSystem* const system = (const VSystem*) vpu->system;
        GET_REG(registers, RA)->as_ptr = system->display;
        *(uint32_t*) (vpu->register_space + RB ) = system->display_width;
        *(uint32_t*) (vpu->register_space + RB4) = system->display_height;
    }
        return 0;
    case VSYS_DISPLAY_UPDATE:{
        VSystem* const system = vpu->system;
        printf("\x1B[2J\x1B[H\n");
        for(uint32_t i = 0; i < system->display_height; i+=1){
            for(uint32_t j = 0; j < system->display_width; j+=1){
                putchar(get_ascii_color(system->display[i * system->display_width + j]));
            }
            putchar('\n');
        }
    }
        return 0;
    
    default:
        return 1;
    } 
}

#endif





#endif // =====================  END OF FILE VSYSTEM_C ===========================