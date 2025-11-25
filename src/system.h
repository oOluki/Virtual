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

    // TODO: threads

    VSYS_NEW_THREAD,
    VSYS_WAIT_THREAD,
    VSYS_DETACH_THREAD,
    VSYS_SLEEP,

    // display

    VSYS_GET_DISPLAY_FRAMEBUFFER,
    VSYS_DISPLAY_UPDATE,

    // for counting putposes
    VSYS_CALL_COUNT
};

int virtual_syscall(VPU* vpu, uint64_t call);

#endif


#endif // =====================  END OF FILE VSYSTEM_HEADER ===========================
