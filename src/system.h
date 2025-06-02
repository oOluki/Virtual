#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER

#include "stdio.h"
#include "core.h"

// in case you want to link a function for the default sys_call method
#ifdef CUSTOM_SYSTEM

#ifdef __cplusplus
extern "C" {
#endif
int sys_call(VPU* vpu, uint64_t call);
#ifdef __cplusplus
}
#endif

#else

enum SysCall{

    SYS_GET_SPECIAL_ADDRESS = 0,
    SYS_GET_SYSTEM_SPECIFICATIONS

};


enum SysSpecialAddress{

    SYS_ADDRESS_STDOUT,
    SYS_ADDRESS_STDERR,
    SYS_ADDRESS_STDIN,

};




int sys_call(VPU* vpu, uint64_t call){

    const uint8_t  inst    = call & 0xFF;
    const uint64_t op_mask = (call & 0xFFFFFFFFFFFFFF00) >> 8;

    switch (inst)
    {
    case SYS_GET_SPECIAL_ADDRESS:
        switch (op_mask)
        {
        case SYS_ADDRESS_STDOUT: vpu->registers[RA / 8].as_ptr = (uint8_t*)stdout; break;
        case SYS_ADDRESS_STDERR: vpu->registers[RA / 8].as_ptr = (uint8_t*)stderr; break;
        case SYS_ADDRESS_STDIN:  vpu->registers[RA / 8].as_ptr = (uint8_t*)stdin;  break;
        default:                 vpu->registers[RA / 8].as_ptr = NULL;   break;
        }
        break;
    case SYS_GET_SYSTEM_SPECIFICATIONS:
        vpu->registers[RA / 8].as_uint64 = is_little_endian();
        break;
    
    default:
        fprintf(stderr, "[ERROR] Invalid Syscall '%u'\n", inst);
        return 1;
    }

    return 0;    
}

#endif


#endif // =====================  END OF FILE SYSTEM_HEADER ===========================
