#ifndef SDL2_VSYSTEM_C
#define SDL2_VSYSTEM_C

#include "system.h"
#include <SDL2/SDL.h>

enum SystemThreadStates{
    SYSTEM_THREAD_STATE_ACTIVE = 1 << 0,
    SYSTEM_THREAD_STATE_DETACHED = 1 << 2,
};

typedef struct _VThread
{
    int             state;
    SDL_Thread*     handle;
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

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  display_canvas;
    uint32_t* display;
    uint32_t  display_width;
    uint32_t  display_height;

} VSystem;

static int _vthread(void* _data){
    _VThread* const thread = (_VThread*) _data;

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

    return 0;
}

// arguments are passed in order from registers ra to rz and returned in order throught the same registers
// \returns 0 on success or non zero error code in failure
int virtual_syscall(VPU* vpu, uint64_t call){

    if(!vpu->system && !(call == VSYS_GET_SYSTEM_SPECIFICATIONS || call == VSYS_INITIALIZE))
        return 1;

    void* const registers = vpu->register_space;

    switch (call)
    {
    case VSYS_GET_SYSTEM_SPECIFICATIONS:
        // _vsystem name goes in RA
        GET_REG(registers, RA)->as_uint64   = *(uint64_t*) "DEFAULT";
        // _vsystem bitsize architecture goes in RB
        GET_REG(registers, RB)->as_uint8    = sizeof(size_t);
        // get _vsystem active state to RC
        GET_REG(registers, RC)->as_uint64   = (vpu->system != NULL);
        // _vsystem endianess goes in RD
        GET_REG(registers, RD)->as_uint8    = is_little_endian()? 1 : 0;
        return 0;
    case VSYS_INITIALIZE:{
        VSystem* const _vsystem = virtual_alloc(sizeof(VSystem));
        if(!_vsystem)
            return 1;

        //memset(_vsystem, 0, sizeof(*_vsystem));

        _vsystem->display = virtual_alloc(GET_REG(registers, RB)->as_uint32 * GET_REG(registers, RB4)->as_uint32 * sizeof(_vsystem->display[0]));
        _vsystem->display_width  = GET_REG(registers, RB )->as_uint32;
        _vsystem->display_height = GET_REG(registers, RB4)->as_uint32;

        if(!_vsystem->display){
            virtual_free(_vsystem);
            return 1;
        }

        if(SDL_Init(SDL_INIT_EVERYTHING)){
            virtual_free(_vsystem->display);
            virtual_free(_vsystem);
            return 1;
        }

        _vsystem->window = SDL_CreateWindow(
            "Virtual",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            _vsystem->display_width, _vsystem->display_height,
            SDL_WINDOW_RESIZABLE
        );

        if(!_vsystem->window){
            virtual_free(_vsystem->display);
            virtual_free(_vsystem);
            SDL_Quit();
            return 1;
        }

        _vsystem->renderer = SDL_CreateRenderer(_vsystem->window, -1, 0);

        if(!_vsystem->renderer){
            virtual_free(_vsystem->display);
            SDL_DestroyWindow(_vsystem->window);
            virtual_free(_vsystem);
            SDL_Quit();
            return 1;
        }

        _vsystem->display_canvas = SDL_CreateTexture(
            _vsystem->renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            _vsystem->display_width, _vsystem->display_height
        );

        if(!_vsystem->display_canvas){
            virtual_free(_vsystem->display);
            SDL_DestroyRenderer(_vsystem->renderer);
            SDL_DestroyWindow(_vsystem->window);
            virtual_free(_vsystem);
            SDL_Quit();
            return 1;
        }

        _vsystem->thread_cap = GET_REG(registers, RA)->as_uint32;
        vpu->system = (void*) _vsystem;
    }
        return 0;
    case VSYS_CLOSE:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        virtual_free(_vsystem->display);
        SDL_DestroyTexture(_vsystem->display_canvas);
        SDL_DestroyRenderer(_vsystem->renderer);
        SDL_DestroyWindow(_vsystem->window);
        SDL_Quit();
        virtual_free(_vsystem);
        vpu->system = NULL;
    }
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
    
    case VSYS_NEW_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        if(_vsystem->thread_count + 1 >= _vsystem->thread_cap){
            GET_REG(registers, RA)->as_ptr = NULL;
            return 0;
        }
        _VThread** const old = _vsystem->threads;
        _vsystem->threads = (_VThread**) virtual_alloc((_vsystem->thread_count + 1) * sizeof(_vsystem->threads[0]));
        if(!_vsystem->threads){
            _vsystem->threads = old;
            GET_REG(registers, RA)->as_ptr = NULL;
            return 0;
        }
        memcpy(_vsystem->threads, old, _vsystem->thread_count * sizeof(_vsystem->threads[0]));
        virtual_free(old);
        _VThread* thread = virtual_alloc(sizeof(*thread));
        memset(thread, 0, sizeof(*thread));
        thread->vpu.program = vpu->program;
        thread->calldepth = 1;
        thread->state = SYSTEM_THREAD_STATE_ACTIVE;
        thread->vpu.static_memory = vpu->static_memory;
        thread->vpu.register_space = (uint8_t*) &thread->registers[0];
        thread->vpu.system = vpu->system;
        *GET_REG(thread->vpu.register_space, RA) = *GET_REG(registers, RB);
        GET_REG(thread->vpu.register_space, RIP)->as_uint64 = GET_REG(registers, RA)->as_uint64;
        thread->handle = SDL_CreateThread(_vthread, NULL, thread);
        if(!thread->handle){
            GET_REG(registers, RA)->as_ptr = NULL;
            return 0;
        }
        GET_REG(registers, RA)->as_ptr = thread;
        _vsystem->threads[_vsystem->thread_count++] = thread;
    }
        return 0;
    case VSYS_WAIT_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        for(int i = 0; i < _vsystem->thread_count; i+=1){
            if(thread == _vsystem->threads[i]){
                GET_REG(vpu->register_space, RA)->as_ptr = NULL;
                SDL_WaitThread(thread->handle, (int*) &GET_REG(vpu->register_space, RA)->as_int64);
                virtual_free(thread);
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
                return 0;
            }
        }
    }
        return 1;
    case VSYS_DETACH_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        for(int i = 0; i < _vsystem->thread_count; i+=1){
            if(thread == _vsystem->threads[i]){
                GET_REG(vpu->register_space, RA)->as_ptr = NULL;
                SDL_DetachThread(thread->handle);
                thread->state |= SYSTEM_THREAD_STATE_DETACHED;
                virtual_free(thread);
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
                return 0;
            }
        }
    }
        return 1;
    case VSYS_SLEEP:
        SDL_Delay(GET_REG(registers, RA)->as_uint32);
        return 0;

    case VSYS_GET_DISPLAY_FRAMEBUFFER:{
        const VSystem* const _vsystem = (const VSystem*) vpu->system;
        GET_REG(registers, RA)->as_ptr = (uint8_t*) _vsystem->display;
        *(uint32_t*) (vpu->register_space + RB ) = _vsystem->display_width;
        *(uint32_t*) (vpu->register_space + RB4) = _vsystem->display_height;
    }
        return 0;
    case VSYS_DISPLAY_UPDATE:{
        VSystem* const _vsystem = vpu->system;
        void* pixels;
        int pitch;
        if(SDL_LockTexture(_vsystem->display_canvas, NULL, &pixels, &pitch)){
            return 1;
        }

        for(int i = 0; i < _vsystem->display_height; i+=1){
            uint32_t* const row =(uint32_t*) ( ((uintptr_t) pixels) + i * pitch);
            for(int j = 0; j < _vsystem->display_width; j+=1){
                row[j] = _vsystem->display[i * _vsystem->display_width + j];
            }
        }

        SDL_UnlockTexture(_vsystem->display_canvas);
        SDL_RenderCopy(_vsystem->renderer, _vsystem->display_canvas, NULL, NULL);
        SDL_RenderPresent(_vsystem->renderer);
    }
        return 0;
    
    default:
        return 1;
    } 
}



#endif // =====================  END OF FILE SDL2_VSYSTEM_C ===========================