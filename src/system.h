#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER

#include "stdio.h"
#include "core.h"
#include "virtual.h"

// if you provide your own sys_call method, you can to do whatever you want on a syscall with any id number
// this is the default and a suggestion...
enum SysCall{

    SYS_GET_SYSTEM_SPECIFICATIONS = 0,
    SYS_INITIALIZE,
    SYS_CLOSE,
    SYS_MEMALLOC,
    SYS_MEMFREE,
    SYS_GET_DISPLAY_FRAMEBUFFER,
    SYS_DISPLAY_UPDATE,

    // for counting putposes
    SYS_CALL_COUNT
};

// if you provide your own sys_call method, you can don't need to follow this
typedef struct System
{
    uint32_t* display;
    uint32_t  display_width;
    uint32_t  display_height;

} System;


// in case you want to link a function for the default sys_call method
#ifdef CUSTOM_SYSTEM

#ifdef __cplusplus
extern "C" {
#else
extern
#endif
// \returns 0 on success or non zero error code in failure
int sys_call(VPU* vpu, uint64_t call);
#ifdef __cplusplus
}
#endif

#else

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
    const int ascii_index = (brightness <= 255)? (brightness * ascii_maplen) / 255 : ascii_maplen - 1;
    return ascii_map[ascii_index];
}

// if you provide your own sys_call method, you can to do whatever you want on a syscall with any id number
// this is the default and a suggestion for what each call hould try to do...
// \returns 0 on success or non zero error code in failure
int sys_call(VPU* vpu, uint64_t call){

    if(!vpu->system && !(call == SYS_GET_SYSTEM_SPECIFICATIONS || call == SYS_GET_SYSTEM_SPECIFICATIONS))
        return 1;

    switch (call)
    {
    case SYS_GET_SYSTEM_SPECIFICATIONS:
        // system name goes in RA
        vpu->registers[RA >> 3].as_ptr   = "VPU_STANDARD_SYSTEM";
        // system bitsize architecture goes in RB
        vpu->registers[RB >> 3].as_uint8 = sizeof(size_t);
        // get system active state to RC
        vpu->registers[RC >> 3].as_uint64 = (vpu->system != NULL);
        // system endianess goes in RE
        vpu->registers[RE >> 3].as_uint8 = is_little_endian()? 1 : 0;
        return 0;
    case SYS_INITIALIZE:{
        if(vpu->system) return 0;
        vpu->system = virtual_alloc(sizeof(System));
        if(!vpu->system)
            return 1;
        ((System*) vpu->system)->display = virtual_alloc_aligned(
            (*(uint32_t*) (vpu->register_space + RA)) * (*(uint32_t*) (vpu->register_space + RA4)),
            4
        );
        if(!(((System*) vpu->system)->display)){
            free(vpu->system);
            vpu->system = NULL;
            return 1;
        }
    }
        return 0;
    case SYS_CLOSE:
        free(vpu->system);
        vpu->system = NULL;
        return 0;
    case SYS_MEMALLOC:
        vpu->registers[RA >> 3].as_ptr = virtual_alloc((size_t) vpu->registers[RA >> 3].as_uint64);
        return 0;
    case SYS_MEMFREE:
        free(vpu->registers[RA >> 3].as_ptr);
        return 0;
    case SYS_GET_DISPLAY_FRAMEBUFFER:{
        System* const system = vpu->system;
        vpu->registers[RA >> 3].as_ptr = (uint8_t*) system->display;
        *(uint32_t*) (vpu->register_space + RB ) = system->display_width;
        *(uint32_t*) (vpu->register_space + RB4) = system->display_height;
    }
        return 0;
    case SYS_DISPLAY_UPDATE:{
        System* const system = vpu->system;
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


#endif // =====================  END OF FILE SYSTEM_HEADER ===========================
