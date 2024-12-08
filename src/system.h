#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER

#include "stdio.h"
#include "core.h"

#include <dlfcn.h>

typedef enum SysCall{

    SYS_GET_SPECIAL_ADDRESS = 0,
    SYS_GET_SYSTEM_SPECIFICATIONS,
    SYS_DLOPEN,
    SYS_DLSYM,
    SYS_DLCLOSE,
    SYS_CALL_EXT,

} SysCall;


enum SysSpecialAddress{

    SYS_ADDRESS_STDOUT,
    SYS_ADDRESS_STDERR,
    SYS_ADDRESS_STDIN,

} SysSpecialAddress;




int sys_call(VPU* vpu, uint64_t call){

    const uint8_t  inst    = call & 0xFF;
    const uint64_t op_mask = (call & 0xFFFFFFFFFFFFFF00) >> 8;

    switch (inst)
    {
    case SYS_GET_SPECIAL_ADDRESS:
        switch (op_mask)
        {
        case SYS_ADDRESS_STDOUT: vpu->registers[RA / 8].as_ptr = stdout; break;
        case SYS_ADDRESS_STDERR: vpu->registers[RA / 8].as_ptr = stderr; break;
        case SYS_ADDRESS_STDIN:  vpu->registers[RA / 8].as_ptr = stdin;  break;
        default:                 vpu->registers[RA / 8].as_ptr = NULL;   break;
        }
        break;
    case SYS_GET_SYSTEM_SPECIFICATIONS:
        vpu->registers[RA / 8].as_uint64 = is_little_endian();
        break;
    case SYS_DLOPEN:
        vpu->registers[RA / 8].as_ptr = dlopen(vpu->registers[RA / 8].as_ptr, op_mask);
        break;
    case SYS_DLSYM:
        vpu->registers[RA / 8].as_ptr = dlsym(vpu->registers[RA / 8].as_ptr, vpu->registers[RB / 8].as_ptr);
        break;
    case SYS_DLCLOSE:
        vpu->registers[RA / 8].as_int32 = dlclose(vpu->registers[RA / 8].as_ptr);
        break;
    case SYS_CALL_EXT:
        #define FUN (vpu->stack[--vpu->registers[RSP / 8].as_uint64])
        #define RAREG vpu->registers[RA / 8].as_uint64
        #define RBREG vpu->registers[RB / 8].as_uint64
        #define RCREG vpu->registers[RC / 8].as_uint64
        #define RDREG vpu->registers[RD / 8].as_uint64
        #define REREG vpu->registers[RE / 8].as_uint64
        #define RFREG vpu->registers[RF / 8].as_uint64
        switch (op_mask)
        {
        case 0: RAREG = ((uint64_t(*)())FUN)(); break;
        case 1: RAREG = ((uint64_t(*)(uint64_t))(FUN))(RBREG); break;
        case 2: RAREG = ((uint64_t(*)(uint64_t, uint64_t))(FUN))(RBREG, RCREG); break;
        case 3: RAREG = ((uint64_t(*)(uint64_t, uint64_t, uint64_t))(FUN))(RBREG, RCREG, RDREG); break;
        case 4: RAREG = ((uint64_t(*)(uint64_t, uint64_t, uint64_t, uint64_t))(FUN))(RBREG, RCREG, RDREG, REREG); break;
        case 5: RAREG = ((uint64_t(*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(FUN))(RBREG, RCREG, RDREG, REREG, RFREG); break;
        default: break;
        }
        #undef FUN
        #undef RAREG
        #undef RBREG
        #undef RCREG
        #undef RDREG
        #undef REREG
        #undef RFREG
        break;
    
    default:
        fprintf(stderr, "[ERROR] Invalid Syscall '%u'\n", inst);
        return 1;
    }

    return 0;    
}


#endif // =====================  END OF FILE SYSTEM_HEADER ===========================