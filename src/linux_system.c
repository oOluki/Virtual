#ifndef VLINUX_SYSTEM_C
#define VLINUX_SYSTEM_C

#include "system.h"
#include "core.h"
#include <pthread.h>

enum SystemThreadStates{
    SYSTEM_THREAD_STATE_ACTIVE = 1 << 0,
    SYSTEM_THREAD_STATE_DETACHED = 1 << 2,
};

typedef struct _VThread
{
    int             state;
    pthread_t       handle;
    pthread_mutex_t mutex;
    void*           shared;
    int             calldepth;
    VPU             vpu;
    Register        registers[REGISTER_SPACE_SIZE / sizeof(Register)];
    uint64_t        stack[1024];
} _VThread;


typedef struct VSystem
{

    _VThread**  threads;
    int         thread_count;
    int         thread_cap;

    uint32_t* display;
    uint32_t  display_width;
    uint32_t  display_height;

} VSystem;

static void* _vthread(void* _data){
    _VThread* const thread = (_VThread*) _data;

    pthread_mutex_lock(&thread->mutex);

    VPU* const vpu = &thread->vpu;

    while((thread->state & SYSTEM_THREAD_STATE_ACTIVE) && thread->calldepth > 0){

        thread->calldepth += (vpu->program[GET_REG(vpu->register_space, RIP)->as_uint64] & 0xFF) == INST_CALL;
        thread->calldepth -= (vpu->program[GET_REG(vpu->register_space, RIP)->as_uint64] & 0xFF) == INST_RET;

        GET_REG(vpu->register_space, RIP)->as_int64 += perform_inst(
            vpu,
            vpu->program[GET_REG(vpu->register_space, RIP)->as_uint64]
        );
    }

    thread->state &= ~SYSTEM_THREAD_STATE_ACTIVE;

    pthread_mutex_unlock(&thread->mutex);

    return _data;
}

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

// if you provide your own sys_call method, you can to do whatever you want on a syscall with any id number
// this is the default and a suggestion for what each call should try to do...
// arguments are passed in order from registers ra to rz and returned in order throught the same registers
// \returns 0 on success or non zero error code in failure
int syscall(VPU* vpu, uint64_t call){

    if(!vpu->system && !(call == SYS_GET_SYSTEM_SPECIFICATIONS || call == SYS_INITIALIZE))
        return 1;

    void* const registers = vpu->register_space;

    switch (call)
    {
    case SYS_GET_SYSTEM_SPECIFICATIONS:
        // system name goes in RA
        GET_REG(registers, RA)->as_uint64   = *(uint64_t*) "DEFAULT";
        // system bitsize architecture goes in RB
        GET_REG(registers, RB)->as_uint8    = sizeof(size_t);
        // get system active state to RC
        GET_REG(registers, RC)->as_uint64   = (vpu->system != NULL);
        // system endianess goes in RD
        GET_REG(registers, RD)->as_uint8    = is_little_endian()? 1 : 0;
        return 0;
    case SYS_INITIALIZE:{
        VSystem* const system = virtual_alloc(sizeof(VSystem));
        if(!system)
            return 1;

        memset(system, 0, sizeof(*system));

        system->display = virtual_alloc(GET_REG(registers, RA)->as_uint32 * GET_REG(registers, RA4)->as_uint32);
        if(!system->display){
            virtual_free(system);
            return 1;
        }
        system->thread_cap = GET_REG(registers, RB)->as_uint32;
        vpu->system = (void*) system;
    }
        return 0;
    case SYS_CLOSE:
        virtual_free(vpu->system);
        vpu->system = NULL;
        return 0;
    
    // heap
    case SYS_MEMALLOC:
        GET_REG(registers, RA)->as_ptr = virtual_alloc((size_t) GET_REG(registers, RA)->as_uint64);
        return 0;
    case SYS_MEMFREE:
        virtual_free(GET_REG(registers, RA)->as_ptr);
        return 0;
    
    // file operations
    case SYS_FOPEN:
        GET_REG(registers, RA)->as_ptr = (uint8_t*) fopen(
            (const char*) GET_REG(registers, RA)->as_ptr,
            (const char*) GET_REG(registers, RB)->as_ptr
        );
        return 0;
    case SYS_FCLOSE:
        GET_REG(registers, RA)->as_int32 = fclose((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case SYS_FSEEK:
        GET_REG(registers, RA)->as_int32 = fseek(
            (FILE*) GET_REG(registers, RA)->as_ptr,
            (long)  GET_REG(registers, RB)->as_int64,
            (int)   GET_REG(registers, RC)->as_int32
        );
        return 0;
    case SYS_FTELL:
        GET_REG(registers, RA)->as_int64 = ftell((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case SYS_FWRITE:
        GET_REG(registers, RA)->as_uint64 = (uint64_t) fwrite(
            (void*)  GET_REG(registers, RA)->as_ptr,
            (size_t) GET_REG(registers, RB)->as_uint64,
            (size_t) GET_REG(registers, RC)->as_uint64,
            (FILE*)  GET_REG(registers, RD)->as_ptr
        );
        return 0;
    case SYS_FREAD:
        GET_REG(registers, RA)->as_uint64 = (uint64_t) fread(
            (void*)  GET_REG(registers, RA)->as_ptr,
            (size_t) GET_REG(registers, RB)->as_uint64,
            (size_t) GET_REG(registers, RC)->as_uint64,
            (FILE*)  GET_REG(registers, RD)->as_ptr
        );
        return 0;
    case SYS_FPUTC:
        GET_REG(registers, RA)->as_int64 = fputc(GET_REG(registers, RA)->as_int8, (FILE*) GET_REG(registers, RB)->as_ptr);
        return 0;
    case SYS_FGETC:
        GET_REG(registers, RA)->as_int64 = fgetc((FILE*) GET_REG(registers, RA)->as_ptr);
        return 0;
    case SYS_IOE:
        GET_REG(registers, RA)->as_ptr = stdin;
        GET_REG(registers, RB)->as_ptr = stdout;
        GET_REG(registers, RC)->as_ptr = stderr;
        return 0;
    
    case SYS_NEW_THREAD:{
        VSystem* const system = (VSystem*) vpu->system;
        if(system->thread_count + 1 >= system->thread_cap){
            GET_REG(registers, RA)->as_ptr = NULL;
            return 0;
        }
        _VThread** const old = system->threads;
        system->threads = (_VThread**) malloc((system->thread_count + 1) * sizeof(_VThread*));
        memcpy(system->threads, old, system->thread_count * sizeof(_VThread*));
        free(old);
        _VThread* thread = system->threads[system->thread_count++];
        memset(thread, 0, sizeof(*thread));
        thread->vpu.system = vpu->system;
        thread->vpu.stack = thread->stack;
        thread->vpu.register_space = (uint8_t*) &thread->registers[0];
        thread->registers[RIP >> 3].as_uint64 = GET_REG(registers, RA)->as_uint64;
        thread->registers[RA  >> 3] = *GET_REG(registers, RB);
        GET_REG(registers, RA)->as_ptr = thread;
        if(pthread_mutex_init(&thread->mutex, NULL)){
            GET_REG(registers, RA)->as_ptr = NULL;
            system->thread_count -= 1;
            return 0;
        }
        if(pthread_create(&thread->handle, NULL, _vthread, (void*) thread)){
            pthread_mutex_destroy(&thread->mutex);
            GET_REG(registers, RA)->as_ptr = NULL;
            system->thread_count -= 1;
            return 0;
        }
    }
        return 0;
    case SYS_WAIT_THREAD:{
        VSystem* const system = (VSystem*) vpu->system;
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        int r = 0;
        int thread_found = 0;
        for(int i = 0; i < system->thread_count; i+=1){
            if(thread == system->threads[i]){
                thread_found = 1;
                r = pthread_join(thread->handle, &GET_REG(vpu->register_space, RA)->as_ptr);
                virtual_free(thread);
                system->threads[i] = system->threads[system->thread_count - 1];
                system->thread_count -= 1;
                break;
            }
        }
        return thread_found? r : 1;
    }
    case SYS_DETACH_THREAD:{
        VSystem* const system = (VSystem*) vpu->system;
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        int thread_found = 0;
        for(int i = 0; i < system->thread_count; i+=1){
            if(thread == system->threads[i]){
                thread_found = 1;
                break;
            }
        }
        if(!thread_found){
            return 1;
        }
        return pthread_detach(thread->handle);
    }
    case SYS_SLEEP:
        return 0;

    case SYS_GET_DISPLAY_FRAMEBUFFER:{
        const VSystem* const system = (const VSystem*) vpu->system;
        GET_REG(registers, RA)->as_ptr = (uint8_t*) system->display;
        *(uint32_t*) (vpu->register_space + RB ) = system->display_width;
        *(uint32_t*) (vpu->register_space + RB4) = system->display_height;
    }
        return 0;
    case SYS_DISPLAY_UPDATE:{
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

#endif // =====================  END OF FILE VLINUX_SYSTEM_C ===========================