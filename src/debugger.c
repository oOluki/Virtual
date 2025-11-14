#ifndef _VDEBUG_HEADER
#define _VDEBUG_HEADER

#include "core.h"
#include "execute.c"
#include "parser.h"
#include "disassembler.c"

enum DebugUserPromptCode{
    DUPC_NONE  = 0,
    DUPC_CLEAR_VIEW,
    DUPC_RESIZE_DISPLAY,
    DUPC_RESTART,
    DUPC_EXIT,
    DUPC_GO,
    DUPC_GO_TILL,
    DUPC_GO_UP_TO,
    DUPC_GO_TILL_END,
    DUPC_PERFORM_INST,
    DUPC_ADD_BREAKPOINT,
    DUPC_REMOVE_BREAKPOINT,
    DUPC_BREAKPOINT,
    DUPC_BREAKPOINT_TO,
    DUPC_DO,
    DUPC_SMOOTH_STEP,
    DUPC_STEP,
    DUPC_STEP_IN,
    DUPC_STEP_OUT,
    DUPC_DISREG,
    DUPC_DISPLAY_INST,
    DUPC_SHOW_LABEL,
    DUPC_SHOW_LABEL_AT,
    DUPC_STACK_DISPLAY,


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

    VPU*        vpu;
    Parser      parser;
    Mc_stream_t stream;
    uint8_t*    signals;
    uint64_t    breakpoint_count;
    Mc_stream_t labels;

    Inst*       program;
    uint64_t    program_size;

    uint8_t     display_size;
} Debugger;


int get_prompt_code(const Token first_prompt_token){
    if(mc_compare_token(first_prompt_token, MKTKN("cl")     , 0))  return DUPC_CLEAR_VIEW;
    if(mc_compare_token(first_prompt_token, MKTKN("clear")  , 0))  return DUPC_CLEAR_VIEW;
    if(mc_compare_token(first_prompt_token, MKTKN("resize") , 0))  return DUPC_RESIZE_DISPLAY;
    if(mc_compare_token(first_prompt_token, MKTKN("restart"), 0))  return DUPC_RESTART;
    if(mc_compare_token(first_prompt_token, MKTKN("exit")   , 0))  return DUPC_EXIT;
    if(mc_compare_token(first_prompt_token, MKTKN("go")     , 0))  return DUPC_GO;
    if(mc_compare_token(first_prompt_token, MKTKN("gotill") , 0))  return DUPC_GO_TILL;
    if(mc_compare_token(first_prompt_token, MKTKN("goupto") , 0))  return DUPC_GO_UP_TO;
    if(mc_compare_token(first_prompt_token, MKTKN("goend")  , 0))  return DUPC_GO_TILL_END;
    if(mc_compare_token(first_prompt_token, MKTKN("break")  , 0))  return DUPC_ADD_BREAKPOINT;
    if(mc_compare_token(first_prompt_token, MKTKN("rmbreak"), 0))  return DUPC_REMOVE_BREAKPOINT;
    if(mc_compare_token(first_prompt_token, MKTKN("swbreak"), 0))  return DUPC_BREAKPOINT;
    if(mc_compare_token(first_prompt_token, MKTKN("swbrkto"), 0))  return DUPC_BREAKPOINT_TO;
    if(mc_compare_token(first_prompt_token, MKTKN("do")     , 0))  return DUPC_DO;
    if(mc_compare_token(first_prompt_token, MKTKN("")       , 0))  return DUPC_SMOOTH_STEP;
    if(mc_compare_token(first_prompt_token, MKTKN("step")   , 0))  return DUPC_STEP;
    if(mc_compare_token(first_prompt_token, MKTKN("stepin") , 0))  return DUPC_STEP_IN;
    if(mc_compare_token(first_prompt_token, MKTKN("stepout"), 0))  return DUPC_STEP_OUT;
    if(mc_compare_token(first_prompt_token, MKTKN("disreg") , 0))  return DUPC_DISREG;
    if(mc_compare_token(first_prompt_token, MKTKN("inst")   , 0))  return DUPC_DISPLAY_INST;
    if(mc_compare_token(first_prompt_token, MKTKN("label")  , 0))  return DUPC_SHOW_LABEL;
    if(mc_compare_token(first_prompt_token, MKTKN("labelat"), 0))  return DUPC_SHOW_LABEL_AT;
    if(mc_compare_token(first_prompt_token, MKTKN("stack")  , 0))  return DUPC_STACK_DISPLAY;

    return DUPC_ERROR;
}

int get_prompt_expected_argc(int code){
    switch (code)
    {
    case DUPC_NONE:
    case DUPC_CLEAR_VIEW:
    case DUPC_EXIT:
    case DUPC_GO:
    case DUPC_GO_TILL_END:
    case DUPC_RESTART:
    case DUPC_SMOOTH_STEP:
    case DUPC_STEP_IN:
    case DUPC_STEP_OUT:
    case DUPC_DO:
        return 0;
    case DUPC_RESIZE_DISPLAY:
    case DUPC_PERFORM_INST:
    case DUPC_ADD_BREAKPOINT:
    case DUPC_REMOVE_BREAKPOINT:
    case DUPC_BREAKPOINT:
    case DUPC_BREAKPOINT_TO:
    case DUPC_STEP:
    case DUPC_SHOW_LABEL:
    case DUPC_SHOW_LABEL_AT:
    case DUPC_STACK_DISPLAY:
    case DUPC_GO_TILL:
    case DUPC_GO_UP_TO:
        return 1;
    case DUPC_DISPLAY_INST:
        return 2;
    default:
        return -1;
    }
}

int debug_display_inst(Debugger* debugger, uint64_t center, uint64_t width){

    VPU* const vpu = debugger->vpu;
    uint64_t i      = (width < center)? center - width : 0;
    uint64_t finish = (center + width < debugger->program_size)? center + width : debugger->program_size;

    char charbuff[30];
    char* buff[3] = {
        &charbuff[0],
        &charbuff[10],
        &charbuff[20]
    };
    int status = 0;
    if(debugger->output == stdout) printf("\x1B[2J\x1B[H");
    fprintf(debugger->output, "inst: %"PRIu64"\tstack: %"PRIu64"\tbreakpoints: %"PRIu64"\n",
        vpu->registers[RIP >> 3].as_uint64, vpu->registers[RSP >> 3].as_uint64, debugger->breakpoint_count);

    for(; i < finish && i < vpu->registers[RIP >> 3].as_uint64 && !status; i+=1){
        if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
        else fprintf(debugger->output, "%3"PRIu64"", i);
        status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
    }
    if(i == vpu->registers[RIP >> 3].as_uint64 && !status){
        fprintf(debugger->output, "*");
        if(i >= debugger->program_size){
            fputc('\n', debugger->output);
        } else{
            if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
            status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i++, buff);
        }
    }
    for(; i < finish && !status; i+=1){
        if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
        else fprintf(debugger->output, "%3"PRIu64"", i);
        status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
    }
    if(status){
        fprintf(debugger->err, "[ERROR] While Displaying Instruction %"PRIu32" At %"PRIu64"\n", debugger->program[i], i);
        return 1;
    }
    return 0;
}
int perform_user_prompt(Debugger* debugger, DebugUserPrompt prompt){
    switch (prompt.code)
    {
    case DUPC_NONE:
        break;
    case DUPC_CLEAR_VIEW:
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        break;
    case DUPC_RESIZE_DISPLAY:
        debugger->display_size = (prompt.arg1.as_uint64 < 100)? prompt.arg1.as_uint64 : 100;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        break;
    case DUPC_RESTART:
        memset(debugger->vpu->register_space, 0, REGISTER_SPACE_SIZE);
        debugger->vpu->registers[RIP >> 3].as_uint64 = debugger->parser.entry_point;
        debugger->vpu->registers[RSP >> 3].as_uint64 = 0;
        debugger->vpu->status = 0;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        break;
    case DUPC_EXIT:
        debugger->is_active = 0;
        break;
    case DUPC_GO:{
        uint64_t i = 0;
        while(
            debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size &&
            !debugger->vpu->status &&
            !(debugger->signals[debugger->vpu->registers[RIP >> 3].as_uint64] & DEBUG_SIGNAL_BREAK_MASK)
        ){

            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_GO_TILL:{
        while(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !debugger->vpu->status &&
            debugger->vpu->registers[RIP >> 3].as_uint64 != prompt.arg1.as_uint64){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }   break;
    case DUPC_GO_UP_TO:{
        const uint64_t maxip = (prompt.arg1.as_uint64 < debugger->program_size)? prompt.arg1.as_uint64 : debugger->program_size;
        while(debugger->vpu->registers[RIP >> 3].as_uint64 < maxip && !debugger->vpu->status ){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }   break;
    case DUPC_GO_TILL_END:
        while(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !debugger->vpu->status){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        fprintf(debugger->output, "Vpu Returned Status %u\n", debugger->vpu->status);
        break;
    case DUPC_PERFORM_INST:{
        const Inst inst = prompt.arg1.as_uint32;
        const uint32_t code = inst & 0xFF;
        if(code == INST_JMP || code == INST_JMPF || code == INST_JMPFN || code == INST_CALL || code == INST_RET){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(debugger->vpu, inst);
            printf("ahoooooo\n");
        }
        else perform_inst(debugger->vpu, inst);
    }   break;
    case DUPC_ADD_BREAKPOINT:{
        if(prompt.arg1.as_uint64 >= debugger->program_size){
            fprintf(
                debugger->err,
                "[ERROR] Can't add break point to %"PRIu64", program is only %"PRIu64" instructions long\n",
                prompt.arg1.as_uint64, debugger->program_size
            );
            return 1;
        }
        if(!(debugger->signals[prompt.arg1.as_uint64] & DEBUG_SIGNAL_BREAK_MASK)) debugger->breakpoint_count+=1;
        debugger->signals[prompt.arg1.as_uint64] |= DEBUG_SIGNAL_BREAK_MASK;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_REMOVE_BREAKPOINT:
        if(prompt.arg1.as_uint64 >= debugger->program_size){
            fprintf(
                debugger->err,
                "[ERROR] No Break Point%"PRIu64", Program Is %"PRIu64" Instructions Long\n",
                prompt.arg1.as_uint64, debugger->program_size
            );
            return 1;
        }
        if(!(debugger->signals[prompt.arg1.as_uint64] & DEBUG_SIGNAL_BREAK_MASK)){
            fprintf(debugger->err, "[ERROR] No Break Point On Instruction At Position %"PRIu64"\n", prompt.arg1.as_uint64);
            return 1;
        }
        debugger->breakpoint_count-=1;
        debugger->signals[prompt.arg1.as_uint64] &= ~DEBUG_SIGNAL_BREAK_MASK;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        break;
    case DUPC_BREAKPOINT:
        uint64_t breakpoints_found = 0;
        uint64_t i = 0;
        if(prompt.arg1.as_uint64 >= debugger->breakpoint_count){
            fprintf(debugger->output, "No %"PRIu64"th breakpoint, there are only %"PRIu64" breakpoints\n", prompt.arg1.as_uint64, debugger->breakpoint_count);
            break;
        }
        for(; i < debugger->program_size && breakpoints_found < prompt.arg1.as_uint64 + 1; i+=1){
            breakpoints_found += ((debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) != 0);
        }
        if(breakpoints_found < prompt.arg1.as_uint64 + 1){
            fprintf(
                debugger->err,
                "[ERROR] " __FILE__ ":%i:0: Found %"PRIu64" breakpoints, where there should be %"PRIu64"\n",
                __LINE__, breakpoints_found, debugger->breakpoint_count
            );
        }
        else{
            fprintf(debugger->output, "%"PRIu64"th breakpoint In %"PRIu64"\n", prompt.arg1.as_uint64, i);
        }
        break;
    case DUPC_BREAKPOINT_TO:{
        if(prompt.arg1.as_uint64 >= debugger->program_size){
            fprintf(
                debugger->output,
                "No breakpoint to %"PRIu64", program is only %"PRIu64" instructions long\n",
                prompt.arg1.as_uint64, debugger->program_size
            );
            if(debugger->breakpoint_count > 0){
                for(uint64_t i = 1; i < debugger->program_size; i+=1){
                    if((debugger->signals[debugger->program_size - i] & DEBUG_SIGNAL_BREAK_MASK)){
                        fprintf(debugger->output, "closest breakpoint at %"PRIu64"\n", debugger->program_size - i);
                    } 
                }
            }
            break;
        }
        else if(!(debugger->signals[prompt.arg1.as_uint64] & DEBUG_SIGNAL_BREAK_MASK)){
            fprintf(debugger->output, "No breakpoint to %"PRIu64"\n", prompt.arg1.as_uint64);
            if(debugger->breakpoint_count > 0){
                uint64_t i = prompt.arg1.as_uint64;
                for(; i < debugger->program_size && !(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK); i+=1);
                uint64_t j = prompt.arg1.as_uint64;
                for(j = prompt.arg1.as_uint64; j > 0 && !(debugger->signals[j] & DEBUG_SIGNAL_BREAK_MASK); j-=1);
                if(!(debugger->signals[j] & DEBUG_SIGNAL_BREAK_MASK)) j = 0;
                if((debugger->signals[j] & DEBUG_SIGNAL_BREAK_MASK) && (prompt.arg1.as_uint64 - j) <= (i - prompt.arg1.as_uint64))
                    i = j;
                fprintf(debugger->output, "closest breakpoint at %"PRIu64"\n", i);
            }
            break;
        }
        fprintf(debugger->output, "breakpoint at %"PRIi64"\n", prompt.arg1.as_uint64);
    }
        break;
    case DUPC_DO:
        if(debugger->output == stdout) printf("\x1B[2J\x1B[H");
        if((debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL){
            VPU* const vpu = debugger->vpu;
            int ret_required_count = 0;
            do {
                ret_required_count += ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL);
                ret_required_count -= ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_RET);

                vpu->registers[RIP >> 3].as_int64 += perform_inst(
                    vpu,
                    debugger->program[vpu->registers[RIP >> 3].as_uint64]
                );

            } while(vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !vpu->status && ret_required_count > 0);
        }
        else if(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        break;
    case DUPC_SMOOTH_STEP:{
        if((debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL){
            VPU* const vpu = debugger->vpu;
            int ret_required_count = 0;
            do {
                ret_required_count += ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL);
                ret_required_count -= ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_RET);

                vpu->registers[RIP >> 3].as_int64 += perform_inst(
                    vpu,
                    debugger->program[vpu->registers[RIP >> 3].as_uint64]
                );

            } while(vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !vpu->status && ret_required_count > 0);
        }
        else if(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size){
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }   break;
    case DUPC_STEP:{
        for(uint64_t i = 0; i < prompt.arg1.as_uint64; i+=1){
            if(((debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL) &&
            debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !debugger->vpu->status){
                VPU* const vpu = debugger->vpu;
                int ret_required_count = 0;
                do {
                    ret_required_count += ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL);
                    ret_required_count -= ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_RET);

                    vpu->registers[RIP >> 3].as_int64 += perform_inst(
                        vpu,
                        debugger->program[vpu->registers[RIP >> 3].as_uint64]
                    );

                } while(vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !vpu->status && ret_required_count > 0);
            }
            else if(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size){
                debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                    debugger->vpu,
                    debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
                );
            }
            else break;
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_STEP_IN:{
        VPU* const vpu = debugger->vpu;
        while(vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !vpu->status &&
            (debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) != INST_CALL){
            vpu->registers[RIP >> 3].as_int64 += perform_inst(
                vpu,
                debugger->program[vpu->registers[RIP >> 3].as_uint64]
            );
        }
        if((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL){
            vpu->registers[RIP >> 3].as_int64 += perform_inst(
                vpu,
                debugger->program[vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_STEP_OUT:{
        VPU* const vpu = debugger->vpu;
        int ret_required_count = 1;
        while(vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !vpu->status && ret_required_count > 0){

            ret_required_count += ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL);
            ret_required_count -= ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_RET);

            vpu->registers[RIP >> 3].as_int64 += perform_inst(
                vpu,
                debugger->program[vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_DISREG:{
        const int reg = get_reg((Token){.value.as_str = (char*) prompt.arg1.as_ptr, .size = (int) prompt.arg2.as_uint32, .type = TKN_RAW});
        if(reg < 0){
            fprintf(debugger->err, "[ERROR] No register '%.*s'\n", (int) prompt.arg2.as_uint32, (char*) prompt.arg1.as_ptr);
            return 1;
        }
        Register r;
        memcpy(&r, debugger->vpu->register_space + reg, sizeof(r));
        char buff[8];
        fprintf(debugger->output, "%s = (%08"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n",
            get_reg_str(reg, buff), r.as_uint64, r.as_uint64, r.as_int64, r.as_float64);
    }
        return 0;
    case DUPC_DISPLAY_INST:
        return debug_display_inst(debugger, prompt.arg1.as_uint64, prompt.arg2.as_uint64);
    case DUPC_SHOW_LABEL:{
        const Label* label_ptr = get_label(
            debugger->parser.labels,
            (Token){.value.as_str = (char*) prompt.arg1.as_ptr, .size = (int) prompt.arg2.as_int32, .type = TKN_STR}
        );
        if(!label_ptr){
            fprintf(debugger->err, "[ERROR] Could Not Find Label '%.*s'\n", (int) prompt.arg2.as_int32, (char*) prompt.arg1.as_ptr);
            return 1;
        }
        const Label label = get_label_from_raw_data(label_ptr);
        fprintf(
            debugger->output,
            "label at %zu of size %"PRIu32":\n"
            "\tname: %.*s\n"
            "\ttype: %"PRIu8"\n"
            "\tdefinition: ",
            (size_t)((uintptr_t)(label_ptr) - (uintptr_t)(debugger->parser.labels->data)), label.size,
            (int) prompt.arg2.as_int32, (char*) prompt.arg1.as_ptr,
            label.type
        );
        if(label.type == TKN_STR || label.type == TKN_RAW){
            fprintf(debugger->output, "%.*s\n", (int)(*(uint64_t*)((uint8_t*)(label_ptr) + label.definition.as_uint)),
                (char*)((uint8_t*)(label_ptr) + label.definition.as_uint));
        }
        else{
            fprintf(debugger->output, "u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n", label.definition.as_uint, label.definition.as_int,
                label.definition.as_float, label.definition.as_str);
        }
    }
        break;
    
    case DUPC_SHOW_LABEL_AT:{
        if(prompt.arg1.as_uint64 >= debugger->labels.size){
            fprintf(debugger->err, "[ERROR] No Label Position %"PRIu64" Out Of Bounds %"PRIu64"\n", prompt.arg1.as_uint64, debugger->labels.size);
            return 1;
        }
        const Label* label_ptr = (Label*)((uint8_t*)(debugger->labels.data) + prompt.arg1.as_uint64);
        const Label label = get_label_from_raw_data(label_ptr);
        fprintf(
            debugger->output,
            "label at %zu of size %"PRIu32":\n"
            "\tname: %.*s\n"
            "\ttype: %"PRIu8"\n"
            "\tdefinition: ",
            (size_t)((uintptr_t)(label_ptr) - (uintptr_t)(debugger->parser.labels->data)), label.size,
            (int) label.str_size, (const char*) ((uint8_t*) (label_ptr) + label.str),
            label.type
        );
        if(label.type == TKN_STR || label.type == TKN_RAW){
            fprintf(debugger->output, "%.*s\n", (int)(*(uint64_t*)((uint8_t*)(label_ptr) + label.definition.as_uint)),
                (char*)((uint8_t*)(label_ptr) + label.definition.as_uint));
        }
        else{
            fprintf(debugger->output, "u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n", label.definition.as_uint, label.definition.as_int,
                label.definition.as_float, label.definition.as_str);
        }
    }
        break;
    case DUPC_STACK_DISPLAY:
        if(prompt.arg1.as_uint64 >= debugger->vpu->registers[RSP >> 3].as_uint64){
            fprintf(debugger->err, "[ERROR] %"PRIu64" Is Out Of Stack's Bounds(%"PRIu64")\n",
                prompt.arg1.as_uint64, debugger->vpu->registers[RSP >> 3].as_uint64);
            return 1;
        } else{
            const Register v = (Register){.as_uint64 = debugger->vpu->stack[prompt.arg1.as_uint64]};
            fprintf(debugger->output, "u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n", v.as_uint64, v.as_int64, v.as_float64, v.as_ptr);
        }
        break;
    default:
        fprintf(debugger->err, "[ERROR] Can't Perform User Prompt: Unknown UserPrompt Code (%i)\n", prompt.code);
        return 1;
    }
    return 0;
}

DebugUserPrompt get_user_prompt(Debugger* debugger){
    
    char buff[100];
    size_t read = 0;

    Register argv[3] = {0, 0, 0};

    const uint64_t begin = debugger->stream.size;

    printf("\n(vpu) ");

    for(int c = fgetc(debugger->input); c && c != '\n'; c = fgetc(stdin)){

        if(c == EOF){
            printf("\n");
            return (DebugUserPrompt){.code = DUPC_EXIT};
        }
        
        buff[read++] = (char) c;
        if(read == 100){
            mc_stream(&debugger->stream, buff, read);
            read = 0;
        }
    }
    mc_stream(&debugger->stream, buff, read);
    mc_stream_str(&debugger->stream, "");

    *(debugger->parser.tokenizer) = (Tokenizer){
        .line = 0, .column = 0,
        .pos  = 0, .data   = (char*) ((uint8_t*) (debugger->stream.data) + begin)
    };
    Token token = get_next_token(debugger->parser.tokenizer);
    const int code = get_prompt_code(token);
    if(code == DUPC_ERROR){
        const InstProfile inst_profile = get_inst_profile(token);
        if(inst_profile.opcode != INST_ERROR){
            const uint64_t program_size__ = debugger->parser.program->size;
            debugger->parser.program->size = debugger->vpu->registers[RIP >> 3].as_uint64 * sizeof(Inst);
            const Inst inst = parse_inst(&debugger->parser, inst_profile, (StringView){.str = token.value.as_str, .size = token.size});
            debugger->parser.program->size = program_size__;
            if(inst == INST_CONTAINER){
                fprintf(debugger->err, "[ERROR] Can't hanlde such instruction yet\n");
                debugger->stream.size = begin;
                return (DebugUserPrompt){.code = DUPC_ERROR};
            }
            debugger->stream.size = begin;
            if(inst == INST_ERROR)
                return (DebugUserPrompt){.code = DUPC_ERROR};
            return (DebugUserPrompt){.code = DUPC_PERFORM_INST, .arg1.as_uint32 = inst};
        }
        debugger->stream.size = begin;
        return (DebugUserPrompt){.code = DUPC_ERROR};
    }
    if(code == DUPC_SHOW_LABEL || code == DUPC_DISREG){
        token = get_next_token(debugger->parser.tokenizer);
        if(token.type == TKN_EMPTY || token.type == TKN_NONE){
            fprintf(stderr, "Expected raw name\n");
            debugger->stream.size = begin;
            return (DebugUserPrompt){.code = DUPC_ERROR};
        }
        if(get_next_token(debugger->parser.tokenizer).type != TKN_NONE){
            if(get_next_token(debugger->parser.tokenizer).type != TKN_NONE);
            fprintf(stderr, "[ERROR] More Arguments Than Expected, Expected 1\n");
            debugger->stream.size = begin;
            return (DebugUserPrompt){.code = DUPC_ERROR};
        }
        debugger->stream.size = begin;
        return (DebugUserPrompt){.code = code, .arg1.as_ptr = token.value.as_str, .arg2.as_int32 = (int32_t) token.size};
    }
    int expect_op_count = get_prompt_expected_argc(code);
    if(expect_op_count < 0){
        debugger->stream.size = begin;
        return (DebugUserPrompt){.code = DUPC_ERROR};
    }
    int i = 0;
    for(; i < expect_op_count; i+=1){
        token = get_next_token(debugger->parser.tokenizer);

        if(token.type == TKN_EMPTY || token.type == TKN_NONE){
            fprintf(stderr, "Expected %i Arguments, Got %i Instead\n", expect_op_count, i);
            debugger->stream.size = begin;
            return (DebugUserPrompt){.code = DUPC_ERROR};
        }

        if(token.type != TKN_RAW){
            fprintf(stderr, "Invalid TokenType %i\n", token.type);
            debugger->stream.size = begin;
            return (DebugUserPrompt){.code = DUPC_ERROR};
        }

        if(token.value.as_str[0] == '-' || is_char_numeric(token.value.as_str[0])){
            const Operand op = parse_op_literal(token);
            if(op.type == TKN_ERROR)
                return (DebugUserPrompt){.code = DUPC_ERROR};
            argv[i] = op.value;
            continue;
        }
        return (DebugUserPrompt){.code = DUPC_ERROR};
    }

    if(get_next_token(debugger->parser.tokenizer).type != TKN_NONE){
        for(i += 1; get_next_token(debugger->parser.tokenizer).type != TKN_NONE; i+=1);
        fprintf(
            stderr,
            "[ERROR] More Arguments Than Expected, Expected %i, Got %i Instead\n",
            expect_op_count, i
        );
        debugger->stream.size = begin;
        return (DebugUserPrompt){.code = DUPC_ERROR};
    }

    debugger->stream.size = begin;
    return (DebugUserPrompt){.code = code, .arg1 = argv[0], .arg2 = argv[1], .arg3 = argv[3]};
}

// debugs raw program and passes argc and argv to the executing program
int debug(const char* exe){

    if(!exe){
        fprintf(stderr, "[ERROR] Expected Input Program Path\n");
        return 1;
    }

    Mc_stream_t stream = (Mc_stream_t){.data = NULL, .size = 0, .capacity = 0};

    if(!read_file(&stream, exe, 1, 0)){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", exe);
        return 2;
    }

    uint64_t flags;
    uint64_t meta_data_size;
    uint64_t entry_point;
    uint32_t padding;

    const uint64_t skip = sizeof(uint32_t) + sizeof(padding) + sizeof(flags) + sizeof(entry_point) + sizeof(meta_data_size);

    void* meta_data = get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags, &padding);

    if(!meta_data){
        fprintf(stderr, "[ERROR] No MetaData In Program '%s'\n", exe);
        mc_destroy_stream(stream);
        return 1;
    }

    Mc_stream_t static_memory = (Mc_stream_t){.data = NULL, .size = 0, .capacity = 0};
    Mc_stream_t labels        = (Mc_stream_t){.data = NULL, .size = 0, .capacity = 0};

    VPU vpu;

    for(size_t i = 0; i + 8 < meta_data_size; ){
        const uint64_t size = *(uint64_t*)((uint8_t*)(meta_data) + i);
        const uint64_t id   = *(uint64_t*)((uint8_t*)(meta_data) + i + sizeof(uint64_t));
        if(size == 0){
            fprintf(stderr, "[ERROR] Corrupted File: Metadata With Block Of Size 0 (%"PRIu64")\n", id);
            mc_destroy_stream(stream);
            return 1;
        }
        else if(id == (is_little_endian()? mc_swap64(0x5354415449433a00) : 0x5354415449433a00)){
            static_memory.data = (uint8_t*) (meta_data) + i;
            static_memory.size = size;
            vpu.static_memory  = (uint8_t*) static_memory.data;
        }
        else if(id == (is_little_endian()? mc_swap64(0x4c4142454c533a00) : 0x4c4142454c533a00)){
            labels.data = (uint8_t*) (meta_data) + i + sizeof(size) + sizeof(id);
            labels.size = (size > sizeof(size) + sizeof(id))? size - sizeof(size) - sizeof(id) : 0;
        }
        i+=size;
    }
    
    const uint64_t program_size = (stream.size - meta_data_size - skip - padding) / 4;

    vpu.program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    Register registers[REGISTER_SPACE_SIZE / sizeof(Register)];
    for(int i = 0; i < REGISTER_SPACE_SIZE / sizeof(Register); i+=1){
        registers[i].as_uint64 = 0;
    }
    registers[RIP >> 3].as_uint64 = entry_point;
    vpu.register_space = (uint8_t*) registers;
    vpu.registers = (Register*) vpu.register_space;

    vpu.stack = (uint64_t*) malloc(1024);

    vpu.status = 0;

    Tokenizer tokenizer = (Tokenizer){
        .data = (char*)((uint8_t*)(stream.data) + sizeof(uint32_t) + *(uint32_t*)(stream.data) + 1),
        .line = 0, .column = 0, .pos = 0
    };
    // for storing temporary data
    Mc_stream_t dstream = mc_create_stream(1000);

    Parser parser;
    parser.file_path = "stdin";
    parser.file_path_size = sizeof("stdin") - sizeof("");
    parser.labels = &labels;
    parser.static_memory = NULL;
    parser.program = &dstream;
    parser.tokenizer = &tokenizer;
    parser.entry_point = entry_point;
    parser.flags = EXEFLAG_NONE;
    parser.macro_if_depth = 0;

    Debugger debugger;
    debugger.input = stdin;
    debugger.output = stdout;
    debugger.err = stderr;
    debugger.is_active = 1;
    debugger.labels = labels;
    debugger.parser = parser;
    debugger.program = vpu.program;
    debugger.program_size = program_size;
    debugger.signals = malloc((size_t) debugger.program_size);
    memset(debugger.signals, 0, (size_t) debugger.program_size);
    debugger.stream = dstream;
    debugger.vpu = &vpu;
    debugger.display_size = 5;

    debug_display_inst(&debugger, debugger.vpu->registers[RIP >> 3].as_uint64, debugger.display_size);

    while(debugger.is_active) {
	    
        const DebugUserPrompt prompt = get_user_prompt(&debugger);

        if(prompt.code == DUPC_ERROR){
            fprintf(debugger.err, "[ERROR] Invalid Prompt\n\n");
            continue;
        }

        const int status = perform_user_prompt(&debugger, prompt);

        if(status){
            fprintf(debugger.err, "[ERROR] err: %i, Invalid Prompt\n\n", status);
        }

    }


    free(vpu.stack);
    mc_destroy_stream(stream);
    free(debugger.signals);
    mc_destroy_stream(debugger.stream);

    return vpu.status;
}



#endif // =====================  END OF FILE _VDEBUG_HEADER ===========================