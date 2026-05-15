#ifndef _VDEBUG_H
#define _VDEBUG_H

#include "core.h"
#include "virtual.h"
#include "parser.h"

enum DebugUserPromptCode{
    DUPC_NONE  = 0,
    DUPC_SHOW_THIS_PROMPT,
    DUPC_DUMP_THIS_PROMPT,
    DUPC_DUMP_STATE,
    DUPC_CLEAR_VIEW,
    DUPC_RESIZE_DISPLAY,
    DUPC_RESTART,
    DUPC_EXIT,
    DUPC_GO,
    DUPC_GO_TILL_END,
    DUPC_PERFORM_INST,
    DUPC_ADD_BREAKPOINT,
    DUPC_REMOVE_BREAKPOINT,
    DUPC_BREAKPOINT,
    DUPC_DO,
    DUPC_STEP,
    DUPC_STEP_IN,
    DUPC_STEP_OUT,
    DUPC_DISREG,
    DUPC_DISPLAY_INST,
    DUPC_INSPECT,
    DUPC_SHOW_LABEL,
    DUPC_STACK_DISPLAY,
    DUPC_HELP,

    //for counting purposes
    DUPC_DUPC_COUNT,

    DUPC_ERROR = 255
};

enum DebbuggerMetaDataMasks{
    DEBUG_SIGNAL_NONE_MASK = 0,
    DEBUG_SIGNAL_BREAK_MASK = 1 << 0
};

typedef struct DebugUserPrompt{

    int         code;
    Register    arg1;
    Register    arg2;
    Register    arg3;
    
} DebugUserPrompt;

typedef struct Debugger
{
    int         is_active;

    FILE*       input;
    FILE*       output;
    FILE*       err;

    int         argc;
    char**      argv;
    VPU*        vpu;
    Parser      parser;
    Mc_stream_t stream;
    uint8_t*    signals;
    uint64_t    breakpoint_count;

    Inst*       program;
    uint64_t    program_size;

    uint8_t     display_size;
} Debugger;


int get_dupc_code(const char* str);

int debugger_help(Debugger* debugger, int dupc);

int debug_display_inst(Debugger* debugger, uint64_t center, uint64_t width);

int perform_user_prompt(Debugger* debugger, int code, int argc, char** argv);

// \returns 0 on success, 1 if stdin was closed, or -1 on failure
int get_user_prompt(Debugger* debugger, int* _argc, char*** _argv);

// debugs raw program and passes argc and argv to the executing program
// takes input from file at _input or stdin if input == NULL
// writes output to provided _output or stdout if output == NULL
// writes errors to _err or stderr if err == NULL
int debug(const char* input_file, int argc, char** argv);



#endif // =====================  END OF FILE _VDEBUG_H ===========================