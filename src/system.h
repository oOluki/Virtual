#ifndef VSYSTEM_HEADER
#define VSYSTEM_HEADER

#include "core.h"


// in case you want to link a function for the syscall method
#ifdef CUSTOM_SYSTEM

#ifdef __cplusplus
extern "C" {
#else
extern
#endif
// \returns 0 on success or non zero error code in failure
int virtual_syscall(VPU* vpu, uint64_t call);
#ifdef __cplusplus
}
#endif

#else

// if you provide your own syscall method, you can to do whatever you want on a syscall with any id number
// this is the default and a suggestion...
enum VSysCall{

    VSYS_GET_SYSTEM_SPECIFICATIONS = 0,
    VSYS_INITIALIZE,
    VSYS_CLOSE,
    VSYS_EXIT,

    // heap

    VSYS_MEMALLOC,
    VSYS_MEMFREE,

    // file operations

    VSYS_FOPEN,
    VSYS_FCLOSE,
    VSYS_FSEEK,
    VSYS_FTELL,
    VSYS_FWRITE,
    VSYS_FREAD,
    VSYS_FPUTC,
    VSYS_FGETC,
    VSYS_IOE,

    // threads

    VSYS_NEW_THREAD,
    VSYS_WAIT_THREAD,
    VSYS_DETACH_THREAD,
    VSYS_KILL_THREAD,
    VSYS_CREATE_MUTEX,
    VSYS_DESTROY_MUTEX,
    VSYS_LOCK_MUTEX,
    VSYS_UNLOCK_MUTEX,
    VSYS_CREATE_COND,
    VSYS_DESTROY_COND,
    VSYS_SIGNAL_COND,
    VSYS_BROADCAST_COND,
    VSYS_WAIT_COND,

    VSYS_SLEEP,
    VSYS_GET_TIME,
    VSYS_POLL_EVENT,

    // display

    VSYS_GET_DISPLAY_FRAMEBUFFER,
    VSYS_DISPLAY_UPDATE,

    // for counting purposes
    VSYS_CALL_COUNT
};

int virtual_syscall(VPU* vpu, uint64_t call);

#endif


#endif // =====================  END OF FILE VSYSTEM_HEADER ===========================
