#ifndef SDL2_VSYSTEM_C
#define SDL2_VSYSTEM_C

#include "system.h"
#include <SDL2/SDL.h>

enum SystemThreadStates{
    SYSTEM_THREAD_STATE_ACTIVE   = 1 << 0,
    SYSTEM_THREAD_STATE_JOINING  = 1 << 1,
    SYSTEM_THREAD_STATE_DETACHED = 1 << 2,
};


typedef struct _VThread
{
    int             state;
    SDL_Thread*     handle;
    SDL_mutex*      imutex;
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
    SDL_mutex*  imutex;

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

    SDL_LockMutex(thread->imutex);

    if(thread->state & SYSTEM_THREAD_STATE_DETACHED) virtual_free(_data);
    else thread->state &= ~SYSTEM_THREAD_STATE_ACTIVE;

    SDL_UnlockMutex(thread->imutex);

    return 0;
}

static int clear_vsystem(VSystem* _vsystem){

    if(!_vsystem) return 1;

    if(_vsystem->threads){
        for(int i = 0; i < _vsystem->thread_count; i+=1){
            SDL_LockMutex(_vsystem->imutex);
            _vsystem->threads[i]->state &= ~SYSTEM_THREAD_STATE_ACTIVE;
            const int skip = _vsystem->threads[i]->state &  SYSTEM_THREAD_STATE_JOINING;
            _vsystem->threads[i]->state |=  SYSTEM_THREAD_STATE_JOINING;
            SDL_UnlockMutex(_vsystem->imutex);
            if(skip){
                continue;
            }
            SDL_WaitThread(_vsystem->threads[i]->handle, NULL);
            virtual_free(_vsystem->threads[i]);
        }
        SDL_Delay(16);
        virtual_free(_vsystem->threads);
    }
    if(_vsystem->imutex)            SDL_DestroyMutex(_vsystem->imutex);
    if(_vsystem->display)           virtual_free(_vsystem->display);
    if(_vsystem->display_canvas)    SDL_DestroyTexture(_vsystem->display_canvas);
    if(_vsystem->renderer)          SDL_DestroyRenderer(_vsystem->renderer);
    if(_vsystem->window)            SDL_DestroyWindow(_vsystem->window);
    virtual_free(_vsystem);

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

        memset(_vsystem, 0, sizeof(*_vsystem));

        _vsystem->thread_cap = GET_REG(registers, RA)->as_uint32;

        _vsystem->threads = (_VThread**) virtual_alloc(_vsystem->thread_cap * sizeof(_VThread*));

        if(!_vsystem->threads){
            clear_vsystem(_vsystem);
            return 1;
        }

        _vsystem->display = virtual_alloc(GET_REG(registers, RB)->as_uint32 * GET_REG(registers, RB4)->as_uint32 * sizeof(_vsystem->display[0]));
        _vsystem->display_width  = GET_REG(registers, RB )->as_uint32;
        _vsystem->display_height = GET_REG(registers, RB4)->as_uint32;

        if(!_vsystem->display){
            clear_vsystem(_vsystem);
            return 1;
        }

        if(SDL_Init(SDL_INIT_EVERYTHING)){
            clear_vsystem(_vsystem);
            return 1;
        }

        if(_vsystem->thread_cap > 0){
            _vsystem->imutex = SDL_CreateMutex();
            if(!_vsystem->imutex){
                clear_vsystem(_vsystem);
                SDL_Quit();
                return 1;
            }
        }

        _vsystem->window = SDL_CreateWindow(
            "Virtual",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            _vsystem->display_width, _vsystem->display_height,
            SDL_WINDOW_RESIZABLE
        );

        if(!_vsystem->window){
            clear_vsystem(_vsystem);
            SDL_Quit();
            return 1;
        }

        _vsystem->renderer = SDL_CreateRenderer(_vsystem->window, -1, 0);

        if(!_vsystem->renderer){
            clear_vsystem(_vsystem);
            SDL_Quit();
            return 1;
        }

        _vsystem->display_canvas = SDL_CreateTexture(
            _vsystem->renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            _vsystem->display_width, _vsystem->display_height
        );

        if(!_vsystem->display_canvas){
            clear_vsystem(_vsystem);
            SDL_Quit();
            return 1;
        }

        vpu->system = (void*) _vsystem;
    }
        return 0;
    case VSYS_CLOSE:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        clear_vsystem(_vsystem);
        SDL_Quit();
        vpu->system = NULL;
    }
        return 0;
    case VSYS_EXIT:{
        exit((int) GET_REG(registers, RA)->as_int32);
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
        SDL_LockMutex(_vsystem->imutex);
        if(_vsystem->thread_count + 1 >= _vsystem->thread_cap){
            GET_REG(registers, RA)->as_ptr = NULL;
            SDL_UnlockMutex(_vsystem->imutex);
            return 1;
        }
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
        thread->vpu.stack = &thread->stack[0];
        thread->stack[GET_REG(thread->vpu.register_space, RSP)->as_uint64++] = GET_REG(registers, RIP)->as_uint64;
        thread->imutex = _vsystem->imutex;
        GET_REG(registers, RA)->as_ptr = thread;
        _vsystem->threads[_vsystem->thread_count++] = thread;
        thread->handle = SDL_CreateThread(_vthread, NULL, thread);
        if(!thread->handle){
            virtual_free(thread);
            _vsystem->thread_count -= 1;
            GET_REG(registers, RA)->as_ptr = NULL;
            SDL_UnlockMutex(_vsystem->imutex);
            return 1;
        }
        SDL_UnlockMutex(_vsystem->imutex);
    }
        return 0;
    case VSYS_WAIT_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        SDL_LockMutex(_vsystem->imutex);
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        for(int i = 0; i < _vsystem->thread_count; i+=1){
            if(thread == _vsystem->threads[i]){
                GET_REG(vpu->register_space, RA)->as_ptr = NULL;
                if(thread->state & SYSTEM_THREAD_STATE_JOINING)
                    break;
                thread->state |= SYSTEM_THREAD_STATE_JOINING;
                SDL_UnlockMutex(_vsystem->imutex);
                int exitcode;
                SDL_WaitThread(thread->handle, &exitcode);
                GET_REG(vpu->register_space, RA)->as_int64 = (int64_t) exitcode;
                SDL_LockMutex(_vsystem->imutex);
                virtual_free(thread);
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
                SDL_UnlockMutex(_vsystem->imutex);
                return 0;
            }
        }
        SDL_UnlockMutex(_vsystem->imutex);
    }
        return 1;
    case VSYS_DETACH_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        SDL_LockMutex(_vsystem->imutex);
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        for(int i = 0; i < _vsystem->thread_count; i+=1){
            if(thread == _vsystem->threads[i]){
                GET_REG(vpu->register_space, RA)->as_ptr = NULL;
                if(thread->state & SYSTEM_THREAD_STATE_JOINING)
                    break;
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
                if(!(thread->state & SYSTEM_THREAD_STATE_ACTIVE)){
                    virtual_free(thread);
                    SDL_UnlockMutex(_vsystem->imutex);
                    return 0;
                }
                thread->state |= SYSTEM_THREAD_STATE_DETACHED;
                SDL_UnlockMutex(_vsystem->imutex);
                SDL_DetachThread(thread->handle);
                return 0;
            }
        }
        SDL_UnlockMutex(_vsystem->imutex);
    }
        return 1;
    case VSYS_KILL_THREAD:{
        VSystem* const _vsystem = (VSystem*) vpu->system;
        SDL_LockMutex(_vsystem->imutex);
        _VThread* const thread = (_VThread*) GET_REG(vpu->register_space, RA)->as_ptr;
        if(thread == NULL){
            for(int i = 0; i < _vsystem->thread_count; i+=1){
                _vsystem->threads[i]->state &= ~SYSTEM_THREAD_STATE_ACTIVE;
                if(_vsystem->threads[i]->state & SYSTEM_THREAD_STATE_JOINING){
                    continue;
                }
                _vsystem->threads[i]->state |=  SYSTEM_THREAD_STATE_JOINING;
                SDL_UnlockMutex(_vsystem->imutex);
                SDL_WaitThread(_vsystem->threads[i]->handle, NULL);
                SDL_LockMutex(_vsystem->imutex);
                virtual_free(_vsystem->threads[i]);
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
            }
            _vsystem->thread_count = 0;
            SDL_UnlockMutex(_vsystem->imutex);
            return 0;
        }
        else for(int i = 0; i < _vsystem->thread_count; i+=1){
            if(thread == _vsystem->threads[i]){
                GET_REG(vpu->register_space, RA)->as_ptr = NULL;
                thread->state &= ~SYSTEM_THREAD_STATE_ACTIVE;
                if(thread->state & SYSTEM_THREAD_STATE_JOINING)
                    break;
                thread->state |=  SYSTEM_THREAD_STATE_JOINING;
                SDL_UnlockMutex(_vsystem->imutex);
                SDL_WaitThread(thread->handle, NULL);
                SDL_LockMutex(_vsystem->imutex);
                virtual_free(thread);
                _vsystem->threads[i] = _vsystem->threads[_vsystem->thread_count - 1];
                _vsystem->thread_count -= 1;
                SDL_UnlockMutex(_vsystem->imutex);
                return 0;
            }
        }
        SDL_UnlockMutex(_vsystem->imutex);
    }
        return 1;
    case VSYS_CREATE_MUTEX:
        GET_REG(vpu->register_space, RA)->as_ptr = SDL_CreateMutex();
        return 0;
    case VSYS_DESTROY_MUTEX:
        SDL_DestroyMutex((SDL_mutex*) GET_REG(vpu->register_space, RA)->as_ptr);
        return 0;
    case VSYS_LOCK_MUTEX:
        if(GET_REG(vpu->register_space, RB)->as_uint8 == 0){
           GET_REG(vpu->register_space, RA)->as_int64 = SDL_LockMutex((SDL_mutex*) GET_REG(vpu->register_space, RA)->as_ptr);
        }
        else{
            const int r = SDL_TryLockMutex((SDL_mutex*) GET_REG(vpu->register_space, RA)->as_ptr);
            GET_REG(vpu->register_space, RA)->as_int64 = (r == SDL_MUTEX_TIMEDOUT)? 1 : r;
        }
        return 0;
    case VSYS_UNLOCK_MUTEX:
        GET_REG(vpu->register_space, RA)->as_int64 = SDL_UnlockMutex((SDL_mutex*) GET_REG(vpu->register_space, RA)->as_ptr);
        return 0;
    case VSYS_CREATE_COND:
        GET_REG(vpu->register_space, RA)->as_ptr = SDL_CreateCond();
        return 0;
    case VSYS_DESTROY_COND:
        SDL_DestroyCond((SDL_cond*) GET_REG(vpu->register_space, RA)->as_ptr);
        return 0;
    case VSYS_SIGNAL_COND:
        GET_REG(vpu->register_space, RA)->as_int64 = SDL_CondSignal((SDL_cond*) GET_REG(vpu->register_space, RA)->as_ptr);
        return 0;
    case VSYS_BROADCAST_COND:
        GET_REG(vpu->register_space, RA)->as_int64 = SDL_CondBroadcast((SDL_cond*) GET_REG(vpu->register_space, RA)->as_ptr);
        return 0;
    case VSYS_WAIT_COND:
        const int r = SDL_CondWaitTimeout(
            (SDL_cond* ) GET_REG(vpu->register_space, RA)->as_ptr,
            (SDL_mutex*) GET_REG(vpu->register_space, RB)->as_ptr,
            (GET_REG(vpu->register_space, RC)->as_int64 < 0)?
                SDL_MUTEX_MAXWAIT
                    :
                (uint32_t) GET_REG(vpu->register_space, RC)->as_int64
        );
        GET_REG(vpu->register_space, RA)->as_int64 = (r == SDL_MUTEX_TIMEDOUT)? 1 : r;
        return 0;

    case VSYS_SLEEP:
        SDL_Delay(GET_REG(registers, RA)->as_uint32);
        return 0;
    case VSYS_GET_TIME:
        GET_REG(registers, RA)->as_uint64 = SDL_GetTicks64();
        return 0;
    case VSYS_POLL_EVENT:{
        SDL_Event event;
        GET_REG(registers, RA)->as_uint32 = 0;
        if(SDL_PollEvent(&event)){
            GET_REG(registers, RA)->as_uint32 = event.type;
        }
    }
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