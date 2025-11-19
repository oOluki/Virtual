#ifndef _VDEBUG_HEADER
#define _VDEBUG_HEADER

#include "core.h"
#include "execute.c"
#include "parser.h"
#include "disassembler.c"

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

// this accounts for escaped characters.
// it is safe to pass biggets_word = NULL if you don't care about that
// \returns how many word there are in str, or -1 if an invalid word is present or str == NULL
static int get_word_count(const char* str, int* biggest_word){
    if(!str) return -1;

    int words = 0;
    int word_len_max = 0;

    for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

    while(*str){

        int len = 0;

        for(; *str != '\0' && *str != ' ' && *str != '\t' && *str != '\n'; str+=1){
            if(*str == '\\'){
                if(*(++str) == '\0'){
                    return -1;
                }
            }
            len += 1;
        }

        for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

        words += 1;
        if(len > word_len_max) word_len_max = len;
    }

    if(biggest_word) *biggest_word = word_len_max;
    return words;
}

// gets next word in str to output, handling escaped characters
// \returns the pointer to the last word in str, or NULL on failure
static char* get_next_word(const char* str, char* output, int* len, int max_len){

    if(!str || !output) return NULL;

    for(; *str == ' ' || *str == '\t' || *str == '\n'; str+=1);

    int _len = 0;

    for(; *str != '\0' && *str != ' ' && *str != '\t' && *str != '\n' && _len < max_len; str+=1){
        if(*str == '\\'){
            if(*(++str) == '\0'){
                return NULL;
            }
        }
        output[_len++] = *str;
    }
    output[_len] = '\0';

    if(len) *len = _len;

    return (char*) str;
}


int get_dupc_code(const char* str){
    if(mc_compare_str(str, "show_this_prompt", 0))
        return DUPC_SHOW_THIS_PROMPT;
    if(mc_compare_str(str, "dump_this_prompt", 0))
        return DUPC_DUMP_THIS_PROMPT;
    if(mc_compare_str(str, "dump_state", 0))
        return DUPC_DUMP_STATE;

    if(mc_compare_str(str, "cl"     , 0))  return DUPC_CLEAR_VIEW;
    if(mc_compare_str(str, "clear"  , 0))  return DUPC_CLEAR_VIEW;
    if(mc_compare_str(str, "resize" , 0))  return DUPC_RESIZE_DISPLAY;
    if(mc_compare_str(str, "restart", 0))  return DUPC_RESTART;
    if(mc_compare_str(str, "exit"   , 0))  return DUPC_EXIT;
    if(mc_compare_str(str, "e"      , 0))  return DUPC_EXIT;
    if(mc_compare_str(str, "quit"   , 0))  return DUPC_EXIT;
    if(mc_compare_str(str, "q"      , 0))  return DUPC_EXIT;
    if(mc_compare_str(str, "go"     , 0))  return DUPC_GO;
    if(mc_compare_str(str, "goend"  , 0))  return DUPC_GO_TILL_END;
    if(mc_compare_str(str, "execute", 0))  return DUPC_PERFORM_INST;
    if(mc_compare_str(str, "exe"    , 0))  return DUPC_PERFORM_INST;
    if(mc_compare_str(str, "break"  , 0))  return DUPC_ADD_BREAKPOINT;
    if(mc_compare_str(str, "rmbreak", 0))  return DUPC_REMOVE_BREAKPOINT;
    if(mc_compare_str(str, "swbreak", 0))  return DUPC_BREAKPOINT;
    if(mc_compare_str(str, "do"     , 0))  return DUPC_DO;
    if(mc_compare_str(str, "step"   , 0))  return DUPC_STEP;
    if(mc_compare_str(str, "stepin" , 0))  return DUPC_STEP_IN;
    if(mc_compare_str(str, "stepout", 0))  return DUPC_STEP_OUT;
    if(mc_compare_str(str, "disreg" , 0))  return DUPC_DISREG;
    if(mc_compare_str(str, "show"   , 0))  return DUPC_DISPLAY_INST;
    if(mc_compare_str(str, "inspect", 0))  return DUPC_INSPECT;
    if(mc_compare_str(str, "inst"   , 0))  return DUPC_INSPECT;
    if(mc_compare_str(str, "label"  , 0))  return DUPC_SHOW_LABEL;
    if(mc_compare_str(str, "stack"  , 0))  return DUPC_STACK_DISPLAY;
    if(mc_compare_str(str, "help"  , 0))   return DUPC_HELP;

    return DUPC_ERROR;
}

int debugger_help(Debugger* debugger, int dupc){
    switch (dupc)
    {
    case DUPC_NONE:
        return 0;
    case DUPC_SHOW_THIS_PROMPT:
        fprintf(
            debugger->output,
            "show_this_prompt <optional: word sequence>:\n"
            "    displays the prompt's arguments, good for internally testing if input system is ok\n"
        );
        return 0;
    case DUPC_DUMP_THIS_PROMPT:
        fprintf(
            debugger->output,
            "dump_this_prompt <output> <optional: word sequence>:\n"
            "    dumps the prompt's arguments to output, good for internally testing if input system is ok\n"
        );
        return 0;
    case DUPC_DUMP_STATE:
        fprintf(
            debugger->output,
            "dump_state <output_file>:\n"
            "    dumps a debugger state view to the end of output_file, for testing reasons\n"
        );
        return 0;
    case DUPC_CLEAR_VIEW:
        fprintf(
            debugger->output,
            "clear <optional: --complete>:\n"
            "    clears the display, if --complete is passed no instructions are displayed\n"
            "    cl is equivalent to this\n"
        );
        return 0;
    case DUPC_RESIZE_DISPLAY:
        fprintf(
            debugger->output,
            "resize <display heigth>:\n"
            "    sets the instruction display height (number of lines)\n"
        );
        return 0;
    case DUPC_RESTART:
        fprintf(
            debugger->output,
            "restart:\n"
            "    restarts vpu\n"
        );
        return 0;
    case DUPC_EXIT:
        fprintf(
            debugger->output,
            "exit:\n"
            "    exits aplication, quit, q and e are equivalent to this\n"
        );
        return 0;
    case DUPC_GO:
        fprintf(
            debugger->output,
            "go --<optional: kwarg>...:\n"
            "    executes the program up untill a breakpoint is encountered\n"
            "    you can use kwag:\n"
            "        upto <position>: to stop once the program is past the given position\n"
            "        to <position>: to stop once the program hits <position>\n"
            "        nobreaks: to ignore breakpoints\n"
        );
        return 0;
    case DUPC_GO_TILL_END:
        fprintf(
            debugger->output,
            "goend:\n"
            "    executes the program untill vpu halts\n"
        );
        return 0;
    case DUPC_PERFORM_INST:
        fprintf(
            debugger->output,
            "execute <assembly instruction>: \n"
            "    executes the <assembly instruction> with the vpu\n"
            "    exe is equivalent to this\n"
        );
        return 0;
    case DUPC_ADD_BREAKPOINT:
        fprintf(
            debugger->output,
            "break <position>...: \n"
            "    adds a breakpoint to each given position\n"
        );
        return 0;
    case DUPC_REMOVE_BREAKPOINT:
        fprintf(
            debugger->output,
            "rmbreak <position>...: \n"
            "    removes breakpoint from each given position\n"
        );
        return 0;
    case DUPC_BREAKPOINT:
        fprintf(
            debugger->output,
            "swbreak <breakpoint_number>... --<optional: at>...:\n"
            "    shows breakpoints with given breakpoint number\n"
            "    a breakpoint number represents how many breakpoints are behind it on the program, basically an order\n"
            "    you can also use --at <position> to show if there's a breakpoint at that position\n"
        );
        return 0;
    case DUPC_DO:
        fprintf(
            debugger->output,
            "do \n"
            "    performs current instruction without displaying anything on top\n"
            "    convinient in case you want to checkout the programs output to stdout\n"
        );
        return 0;
    case DUPC_STEP:
        fprintf(
            debugger->output,
            "step <optional: count>:\n"
            "    steps count times, or only once if count is not provided\n"
        );
        return 0;
    case DUPC_STEP_IN:
        fprintf(
            debugger->output,
            "stepin:\n"
            "    steps in to a function call\n"
            "    if current instruction is not a function call it will step untill it finds one and then step in\n"
        );
        return 0;
    case DUPC_STEP_OUT:
        fprintf(
            debugger->output,
            "stepout:\n"
            "    steps out of function, by stepping untill it returns from it\n"
        );
        return 0;
    case DUPC_DISREG:
        fprintf(
            debugger->output,
            "disreg <optional: registers>...:\n"
            "    displays provided registers, or all vpu registers if none are provided\n"
        );
        return 0;
    case DUPC_DISPLAY_INST:
        fprintf(
            debugger->output,
            "show <center> <height>:\n"
            "    displays instructions in program from position (center - height / 2) to (center + height / 2)\n"
        );
        return 0;
    case DUPC_INSPECT:
        fprintf(
            debugger->output,
            "inspect <machine code>...:\n"
            "    displays instructions in assembly text that corresponds to the respective provided machine code\n"
            "    if no machine code is provided then all cases will be displayed\n"
            "    inst is equivalent to this\n"
        );
        return 0;
    case DUPC_SHOW_LABEL:
        fprintf(
            debugger->output,
            "label <optional: labels>... --<optional: kwag>...:\n"
            "    displays provided labels, if they exist\n"
            "    use --at <label_memposition> to display a label at that memory offset position from label stream beginning\n"
        );
        return 0;
    case DUPC_STACK_DISPLAY:
        fprintf(
            debugger->output,
            "stack <optional: stack_position>...:\n"
            "    displays elements in stack in the given positions, or the whole stack if none are provided\n"
        );
        return 0;
    case DUPC_HELP:
        fprintf(
            debugger->output,
            "help <optional: what>...:\n"
            "    displays a help message about \"what\", or a general help message if nothing is provided\n"
        );
        return 0;
    
    default:
        return 1;
    }
}

static inline int get_labelpos_to(uint64_t* pos, const char** str, const Mc_stream_t* labels, uint64_t ip){

    for(uint64_t i = 0; i < labels->size; ){
        const Label* const labelptr = (const Label*) (((uint8_t*) labels->data) + i);
        const Label label = get_label_from_raw_data(labelptr);

        if(label.type == TKN_INST_POSITION && label.definition.as_uint >= ip){
            if(pos) *pos = label.definition.as_uint;
            if(str) *str = (const char*) (((uint8_t*) labelptr) + label.str);
            return 1;
        }

        if(label.size == 0){
            return -1;
        }
        i += label.size;
    }
    return 0;
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

    int digit_len_max = 1;
    for(uint64_t psize = debugger->program_size; psize / 10; psize /= 10) digit_len_max += 1;

    uint64_t    label_ip;
    const char* label_ip_str = NULL;
    int         found_label_ip = get_labelpos_to(&label_ip, &label_ip_str, &debugger->labels, i) > 0;

    if(debugger->output == stdout) printf("\x1B[2J\x1B[H\n");
    fprintf(
        debugger->output,
        "inst: %"PRIu64" / %"PRIu64"    stack: %"PRIu64"  breakpoints: %"PRIu64"  status: %i\n",
        vpu->registers[RIP >> 3].as_uint64,
        debugger->program_size,
        vpu->registers[RSP >> 3].as_uint64,
        debugger->breakpoint_count,
        debugger->vpu->status
    );
    for(; i < finish && i < vpu->registers[RIP >> 3].as_uint64; i+=1){
        for(uint64_t lip = i + 1; found_label_ip && i == label_ip && i < finish; lip+=1){
            fprintf(debugger->output, "%s:\n", label_ip_str);
            found_label_ip = get_labelpos_to(&label_ip, &label_ip_str, &debugger->labels, lip) > 0;
            width = (width > 0)? width - 1 : width;
            finish = (center + width < debugger->program_size)? center + width : debugger->program_size;
        }
        if(i >= finish) break;
        if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
        else fprintf(debugger->output, "%*"PRIu64"- ", digit_len_max, i);
        print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
    }
    for(uint64_t lip = i + 1; found_label_ip && i == label_ip && i < finish; lip+=1){
        fprintf(debugger->output, "%s:\n", label_ip_str);
        found_label_ip = get_labelpos_to(&label_ip, &label_ip_str, &debugger->labels, lip) > 0;
        width = (width > 0)? width - 1 : width;
        finish = (center + width < debugger->program_size)? center + width : debugger->program_size;
    }
    if(i == vpu->registers[RIP >> 3].as_uint64 && i < finish){
        fprintf(debugger->output, "*");
        if(i >= debugger->program_size){
            fputc('\n', debugger->output);
        } else{
            if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
            print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i++, buff);
        }
    }
    for(; i < finish; i+=1){
        for(uint64_t lip = i + 1; found_label_ip && i == label_ip && i < finish; lip+=1){
            fprintf(debugger->output, "%s:\n", label_ip_str);
            found_label_ip = get_labelpos_to(&label_ip, &label_ip_str, &debugger->labels, lip) > 0;
            width = (width > 0)? width - 1 : width;
            finish = (center + width < debugger->program_size)? center + width : debugger->program_size;
        }
        if(i >= finish) break;
        if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK) fprintf(debugger->output, "!");
        else fprintf(debugger->output, "%*"PRIu64"- ", digit_len_max, i);
        print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
    }
    return 0;
}

int perform_user_prompt(Debugger* debugger, int code, int argc, char** argv){

    #define EXPECT_ARGC(expected_argc)\
        if((argc - 1) != (expected_argc)){\
            fprintf(stderr, "[ERROR] command %s expects %i arguments, got %i instead\n", argv[0], expected_argc, argc - 1);\
            return 1;\
        }
    #define EXPECT(ARG, TYPE, OUTPUT)\
        const Operand OUTPUT = parse_op_literal(get_token_from_cstr(ARG));\
        if(OUTPUT.type != TYPE){\
            fprintf(stderr, "[ERROR] command %s expected " #TYPE ", got '%s' instead\n", argv[0], ARG);\
            return 1;\
        }

    #define DEBUG_DISPLAY_CENTER_INST(POS, WIDTH) ()

    switch (code)
    {
    case DUPC_NONE:
        break;
    case DUPC_SHOW_THIS_PROMPT:
        for(int i = 0; i < argc; i+=1){
            fprintf(debugger->output, "%i- '%s'\n", i, argv[i]);
        }
        break;
    case DUPC_DUMP_THIS_PROMPT:{
        if(argc <= 1){
            fprintf(debugger->output, "%s expects at least one argument, path to output\n", argv[0]);
            return 1;
        }
        FILE* where = fopen(argv[1], "a");
        if(!where){
            fprintf(debugger->output, "could not open '%s'\n", argv[1]);
            return 1;
        }
        for(int i = 0; i < argc; i+=1){
            fprintf(where, "%i- '%s'\n", i, argv[i]);
        }
        fclose(where);
    }
        break;
    case DUPC_DUMP_STATE:{
        EXPECT_ARGC(1);
        FILE* where = fopen(argv[1], "a");
        if(!where){
            fprintf(debugger->output, "could not open '%s'\n", argv[1]);
            return 1;
        }
        const uint64_t ip = debugger->vpu->registers[RIP >> 3].as_uint64;
        const uint64_t sp = debugger->vpu->registers[RSP >> 3].as_uint64;
        fprintf(where, "display height: %"PRIu8"\n", debugger->display_size);
        fprintf(
            where,
            "inst: %"PRIu64"   stack: %"PRIu64"   breakpoints: %"PRIu64 "    status: %i\n",
            ip, sp, debugger->breakpoint_count, debugger->vpu->status
        );
        fprintf(where, "breakpoints:\n");
        for(uint64_t i = 0; i < debugger->program_size; i+=1){
            if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK){
                fprintf(where, "%"PRIu64"\n", i);
            }
        }
        fprintf(where, "stack:\n");
        for(uint64_t i = 0; i < sp; i+=1){
            fprintf(where, "%"PRIu64"- %"PRIu64"\n", i, debugger->vpu->stack[i]);
        }
        fprintf(where, "registers:\n");
        for(int i = 0; i < REGISTER_SPACE_SIZE / sizeof(Register); i+=1){
            char buff[8];
            fprintf(
                where,
                "%s.as_uint64 = %"PRIu64"\n",
                get_reg_str(i * ((int) sizeof(Register)), buff),
                debugger->vpu->registers[i].as_uint64
            );
        }

        fclose(where);
    }
        break;
    case DUPC_CLEAR_VIEW:
        if(argc > 1){
            for(int i = 1; i < argc; i+=1){
                if(mc_compare_str(argv[i], "--complete", 0)){
                    continue;
                }
                else{
                    fprintf(debugger->err, "[ERROR] %s does not support %s\n", argv[0], argv[i]);
                    return 1;
                }
            }
            fprintf(debugger->output, "\x1B[2J\x1B[H\n");
        }
        else debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        break;
    case DUPC_RESIZE_DISPLAY:{
        EXPECT_ARGC(1);
        EXPECT(argv[1], TKN_ULIT, size);
        if(size.value.as_uint64 > 255){
            fprintf(debugger->output, "max display height is 255 lines\n");
            return 1;
        }
        debugger->display_size = (size.value.as_uint64 < 100)? (uint8_t) size.value.as_uint64 : 100;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_RESTART:{
        memset(debugger->vpu->register_space, 0, REGISTER_SPACE_SIZE);
        debugger->vpu->registers[RIP >> 3].as_uint64 = debugger->parser.entry_point;
        debugger->vpu->registers[RSP >> 3].as_uint64 = 0;
        debugger->vpu->status = 0;
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_EXIT:
        debugger->is_active = 0;
        break;
    case DUPC_GO:{
        Operand position;
        int go_up_to_position = 0;
        int go_till_position  = 0;
        int nobreaks          = 0;
        for(int i = 1; i < argc; i+=1){
            if(mc_compare_str(argv[i], "--to", 0)){
                if(i + 1 >= argc){
                    fprintf(stderr, "[ERROR] %s expects instruction position after %s\n", argv[0], argv[i]);
                    return 1;
                }
                position = parse_op_literal(get_token_from_cstr(argv[i + 1]));
                if(position.type != TKN_ULIT){
                    fprintf(stderr, "[ERROR] %s expects valid instruction position after %s, got '%s' instead\n", argv[0], argv[i], argv[i + 1]);
                    return 1;
                }
                i+=1;
                go_till_position = 1;
            }
            else if(mc_compare_str(argv[i], "--upto", 0)){
                if(i + 1 >= argc){
                    fprintf(stderr, "[ERROR] %s expects instruction position after %s\n", argv[0], argv[i]);
                    return 1;
                }
                position = parse_op_literal(get_token_from_cstr(argv[i + 1]));
                if(position.type != TKN_ULIT){
                    fprintf(stderr, "[ERROR] %s expects valid instruction position after %s, got '%s' instead\n", argv[0], argv[i], argv[i + 1]);
                    return 1;
                }
                i+=1;
                go_up_to_position = 1;
            }
            else if(mc_compare_str(argv[i], "--nobreaks", 0)){
                nobreaks = 1;
            }
            else{
                fprintf(debugger->err, "[ERROR] %s does not hanlde %s\n", argv[0], argv[i]);
                return 1;
            }
        }
        while(debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !debugger->vpu->status){

            if(!nobreaks && (debugger->signals[debugger->vpu->registers[RIP >> 3].as_uint64] & DEBUG_SIGNAL_BREAK_MASK))
                break;
            if(go_till_position && debugger->vpu->registers[RIP >> 3].as_uint64 == position.value.as_uint64)
                break;
            if(go_up_to_position && debugger->vpu->registers[RIP >> 3].as_uint64 >= position.value.as_uint64)
                break;

            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
        debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
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
        if(argc <= 1){
            fprintf(debugger->output, "command %s expects a valid assembly instruction and, therefore, at least one argument\n", argv[0]);
            return 1;
        }

        for(int i = 1; i < argc - 1; i+=1){
            int str_len = 0;
            for(; argv[i][str_len]; str_len+=1);
            argv[i][str_len] = ' ';
        }
        debugger->parser.tokenizer->pos = 0;
        debugger->parser.tokenizer->data = argv[1];
        debugger->parser.tokenizer->column = 0;
        debugger->parser.tokenizer->line = 0;
        const Token inst_tkn = get_next_token(debugger->parser.tokenizer);
        const InstProfile inst_profile = get_inst_profile(inst_tkn);

        if(inst_profile.opcode == INST_ERROR){
            fprintf(
                debugger->output,
                "no valid assembly instruction starts with '%.*s\n'",
                inst_tkn.size, inst_tkn.value.as_str
            );
            return 1;
        }

        const Inst inst = parse_inst(
            &debugger->parser, inst_profile,
            (StringView){.str = inst_tkn.value.as_str, .size = inst_tkn.size}
        );
        switch (inst & 0xFF)
        {
        case INST_ERROR:
            fprintf(debugger->output, "failed to parse '%s'\n", argv[1]);
            return 1;
        case INST_LOAD1:
        case INST_LOAD2:
            fprintf(debugger->output, "'%s' instruction not supported in debug's %s command\n", argv[1], argv[0]);
            return 1;
        case INST_JMP:
        case INST_JMPF:
        case INST_JMPFN:
        case INST_CALL:
        case INST_RET:
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(debugger->vpu, inst);
            return 0;
        default:
            perform_inst(debugger->vpu, inst);
            return 0;
        }
    }   break;
    case DUPC_ADD_BREAKPOINT:
        if(argc <= 1){
            fprintf(debugger->output, "command %s expects at least one breakpoint to add\n", argv[0]);
            return 1;
        }
        for(int i = 1; i < argc; i+=1){
            EXPECT(argv[i], TKN_ULIT, breakpoint_position);
            if(breakpoint_position.value.as_uint64 >= debugger->program_size){
                fprintf(
                    debugger->output,
                    "can't add breakpoint to %"PRIu64", program is only %"PRIu64" instructions long\n",
                    breakpoint_position.value.as_uint64, debugger->program_size
                );
                continue;
            }
            if(!(debugger->signals[breakpoint_position.value.as_uint64] & DEBUG_SIGNAL_BREAK_MASK))
                debugger->breakpoint_count += 1;
            debugger->signals[breakpoint_position.value.as_uint64] |= DEBUG_SIGNAL_BREAK_MASK;
            debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        }
        break;
    case DUPC_REMOVE_BREAKPOINT:{
        if(argc <= 1){
            for(uint64_t i = 0; i < debugger->program_size; i+=1){
                debugger->signals[i] &= ~DEBUG_SIGNAL_BREAK_MASK;
            }
            debugger->breakpoint_count = 0;
            debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
            break;
        }
        uint64_t remove_breakpoints_errors = 0;
        for(int i = 1; i < argc; i+=1){
            EXPECT(argv[i], TKN_ULIT, breakpoint);
            if(breakpoint.value.as_uint64 >= debugger->program_size){
                fprintf(
                    debugger->output,
                    "no breakpoint at %"PRIu64", program is %"PRIu64" instructions long\n",
                    breakpoint.value.as_uint64, debugger->program_size
                );
                remove_breakpoints_errors += 1;
                continue;
            }
            if(!(debugger->signals[breakpoint.value.as_uint64] & DEBUG_SIGNAL_BREAK_MASK)){
                fprintf(debugger->output, "no breakpoint at %"PRIu64" to remove\n", breakpoint.value.as_uint64);
                remove_breakpoints_errors += 1;
                continue;
            }
            if(debugger->breakpoint_count > 0) debugger->breakpoint_count -= 1;
            debugger->signals[breakpoint.value.as_uint64] &= ~DEBUG_SIGNAL_BREAK_MASK;
        }
        if(!remove_breakpoints_errors)
            debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
    }
        break;
    case DUPC_BREAKPOINT:{
        uint64_t breakpoint_number = 0;
        if(argc <= 1){
            if(debugger->breakpoint_count == 0){
                fprintf(debugger->output, "no break points\n");
            }
            else for(uint64_t i = 0; i < debugger->program_size; i+=1){
                if(debugger->signals[i] & DEBUG_SIGNAL_BREAK_MASK){
                    fprintf(debugger->output, "breakpoint number %"PRIu64" at %"PRIu64"\n", breakpoint_number++, i);
                }
            }
            break;
        }
        for(int i = 1; i < argc; i+=1){
            if(mc_compare_str(argv[i], "--at", 0)){
                if(++i >= argc){
                    fprintf(debugger->err, "[ERROR] %s expects instruction position after %s\n", argv[0], argv[i - 1]);
                    continue;
                }
                EXPECT(argv[i], TKN_ULIT, breakpoint);
                if(breakpoint.value.as_uint64 >= debugger->program_size){
                    fprintf(
                        debugger->output,
                        "no breakpoint at %"PRIu64", program is %"PRIu64" instructions long\n",
                        breakpoint.value.as_uint64, debugger->program_size
                    );
                    continue;
                }
                if(!(debugger->signals[breakpoint.value.as_uint64] & DEBUG_SIGNAL_BREAK_MASK)){
                    fprintf(debugger->output, "no breakpoint at %"PRIu64"\n", breakpoint.value.as_uint64);
                    continue;
                }
                continue;
            }
            EXPECT(argv[i], TKN_ULIT, breakpoint);
            breakpoint_number = 0;
            if(breakpoint.value.as_uint64 >= debugger->breakpoint_count){
                fprintf(
                    debugger->output,
                    "No breakpoint number %"PRIu64", there are only %"PRIu64" breakpoints\n",
                    breakpoint.value.as_uint64, debugger->breakpoint_count
                );
                continue;
            }
            uint64_t j = 0;
            for(; j < debugger->program_size && breakpoint_number < breakpoint.value.as_uint64 + 1; j+=1){
                if(debugger->signals[j] & DEBUG_SIGNAL_BREAK_MASK){
                    breakpoint_number += 1;
                }
            }
            if(breakpoint_number == breakpoint.value.as_uint64 + 1){
                fprintf(debugger->output, "breakpoint number %"PRIu64" at %"PRIu64"\n", breakpoint_number, j);
            }
            else{
                fprintf(debugger->err, "[ERROR] expected to find breakpoint number %"PRIu64", but found none\n", breakpoint_number);
            }
        }
    }
        break;
    case DUPC_DO:{
        if(argc > 2){
            fprintf(debugger->err, "[ERROR] %s expects at most one argument, the number of steps, got %i instead\n", argv[0], argc - 1);
            return 1;
        }
        uint64_t steps = 1;
        if(argc == 2){
            EXPECT(argv[1], TKN_ULIT, op);
            steps = op.value.as_uint64;
        }
        if(debugger->vpu->registers[RIP >> 3].as_uint64 >= debugger->program_size) return 0;
        for(uint64_t i = 0; i < steps; i+=1){
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
        }
    }
        break;
    case DUPC_STEP:{
        if(argc > 2){
            fprintf(debugger->err, "[ERROR] %s expects at most one argument, the number of steps, got %i instead\n", argv[0], argc - 1);
            return 1;
        }
        uint64_t steps = 1;
        if(argc == 2){
            EXPECT(argv[1], TKN_ULIT, op);
            steps = op.value.as_uint64;
        }
        if(debugger->vpu->registers[RIP >> 3].as_uint64 >= debugger->program_size)
            return debug_display_inst(debugger, debugger->vpu->registers[RIP >> 3].as_uint64, debugger->display_size);
        for(uint64_t i = 0; i < steps; i+=1){
            if(((debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL) && !debugger->vpu->status){
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
    case DUPC_DISREG:
        if(argc <= 1){
            for(int i = 0; i < REGISTER_SPACE_SIZE / sizeof(Register); i+=1){
                char buff[8];
                const Register r = debugger->vpu->registers[i];
                fprintf(
                    debugger->output,
                    "%.*s = (%08"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n",
                    8, get_reg_str(i * 8, buff),
                    r.as_uint64, r.as_uint64, r.as_int64, r.as_float64
                );
            }
            break;
        }
        for(int i = 1; i < argc; i+=1){
            const int reg = get_reg(get_token_from_cstr(argv[i]));
            if(reg < 0){
                fprintf(debugger->err, "[ERROR] No register '%s'\n", argv[i]);
                return 1;
            }
            Register r;
            memcpy(&r, debugger->vpu->register_space + reg, sizeof(r));
            char buff[8];
            fprintf(debugger->output, "%.*s = (%08"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n",
                8, get_reg_str(reg, buff), r.as_uint64, r.as_uint64, r.as_int64, r.as_float64);
        }
        break;
    case DUPC_DISPLAY_INST:{
        EXPECT_ARGC(2);
        EXPECT(argv[1], TKN_ULIT, center);
        EXPECT(argv[2], TKN_ULIT, width);
        debug_display_inst(debugger, center.value.as_uint64, width.value.as_uint64);
    }
        break;;
    case DUPC_INSPECT:{
        int digit_len_max = 1;
        for(int i = INST_TOTAL_COUNT; i / 10; i /= 10) digit_len_max += 1;
        if(argc <= 1){
            fprintf(debugger->output, "all instructions will have immediate values (literals or registers) zeroed\n");
            fprintf(debugger->output, "instructions of kind LOAD will have containers zeroed\n");
            for(int i = 0; i < INST_TOTAL_COUNT; i+=1){
                char _buff[24];
                char* buff[3] = {&_buff[0], &_buff[8], &_buff[16]};
                const Inst little_program[4] = {i, INST_CONTAINER, INST_CONTAINER, INST_CONTAINER};
                const uint64_t dummy_static[2] = {0, 0};
                fprintf(debugger->output, "%*i-> ", digit_len_max, i);
                print_inst(debugger->output, little_program, (const uint8_t*) dummy_static, 0, buff);
            }
            break;
        }
        for(int i = 1; i < argc; i+=1){
            EXPECT(argv[i], TKN_ULIT, _inst);
            const Inst inst = (Inst) _inst.value.as_uint64;
            if(inst != _inst.value.as_uint64){
                fprintf(
                    debugger->output,
                    "%"PRIu64" is not 4 bytes long and as such can not represent machine code instruction\n",
                    _inst.value.as_uint64
                );
                continue;
            }
            if((inst & 0xFF) == INST_LOAD1 && (inst & 0xFF) == INST_LOAD2){
                fprintf(debugger->output, "instruction of kind LOAD, containers will be zeroed as they can't be infered\n");
            }
            char _buff[24];
            char* buff[3] = {&_buff[0], &_buff[8], &_buff[16]};
            const Inst little_program[4] = {inst, INST_CONTAINER, INST_CONTAINER, INST_CONTAINER};
            const uint64_t dummy_static[2] = {0, 0};
            fprintf(debugger->output, "op code: %*i    machine code: %8"PRIx32"\n", digit_len_max, (int) (inst & 0xFF), inst);
            print_inst(debugger->output, little_program, (const uint8_t*) dummy_static, 0, buff);
        }
    }
        break;;
    case DUPC_SHOW_LABEL:{
        if(argc <= 1){
            for(uint64_t i = 0; i < debugger->labels.size; ){
                const Label* label_ptr = debugger->labels.data;
                const Label  label = get_label_from_raw_data(label_ptr);
                fprintf(
                    debugger->output,
                    "label at %"PRIu64" of size %"PRIu32":\n"
                    "\tname: %.*s\n"
                    "\ttype: %"PRIu8" \'%s\'\n"
                    "\tdefinition: ",
                    i, label.size,
                    (int) label.str_size, (char*) (get_label_name(label_ptr)),
                    label.type, get_token_type_str(label.type)
                );
                if(label.type == TKN_STR || label.type == TKN_RAW){
                    fprintf(debugger->output, "%.*s\n", (int)(*(uint64_t*)((uint8_t*)(label_ptr) + label.definition.as_uint)),
                        (char*)((uint8_t*)(label_ptr) + label.definition.as_uint));
                }
                else{
                    fprintf(debugger->output, "u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n", label.definition.as_uint, label.definition.as_int,
                        label.definition.as_float, label.definition.as_str);
                }
                if(label.size == 0){
                    fprintf(debugger->err, "[ERROR] label at %"PRIu64" has size 0\n", i);
                    return 1;
                }
                i += label.size;
            }
            break;
        }
        for(int i = 1; i < argc; i+=1){
            Label* label_ptr;
            if(mc_compare_str(argv[i], "--at", 0)){
                if(++i >= argc){
                    fprintf(debugger->err, "[ERROR] %s expects label position after %s\n", argv[0], argv[i]);
                    return 1;
                }
                EXPECT(argv[i], TKN_ULIT, label_position);
                if(label_position.value.as_uint64 >= debugger->labels.size){
                    fprintf(debugger->output, "No label at %"PRIu64", labels total size = %"PRIu64"\n",
                        label_position.value.as_uint64, debugger->labels.size);
                }
                label_ptr = (Label*) (((uintptr_t) debugger->labels.data) + label_position.value.as_uint64);
            }
            else{
                label_ptr = get_label(debugger->parser.labels, get_token_from_cstr(argv[i]));
            }
            if(!label_ptr){
                fprintf(debugger->output, "Could not find label '%s'\n", argv[i]);
                continue;
            }
            const Label label = get_label_from_raw_data(label_ptr);
            fprintf(
                debugger->output,
                "label at %"PRIuPTR" of size %"PRIu32":\n"
                "\tname: %.*s\n"
                "\ttype: %"PRIu8" \'%s\'\n"
                "\tdefinition: ",
                ((uintptr_t) label_ptr) - ((uintptr_t) debugger->labels.data), label.size,
                (int) label.str_size, (char*) (get_label_name(label_ptr)),
                label.type, get_token_type_str(label.type)
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
    }
        break;
    case DUPC_STACK_DISPLAY:{
        if(argc <= 1){
            if(debugger->vpu->registers[RSP >> 3].as_uint64 == 0){
                fprintf(debugger->output, "stack is empty\n");
                break;
            }
            for(uint64_t i = 0; i < debugger->vpu->registers[RSP >> 3].as_uint64; i+=1){
                const Register v = (Register){.as_uint64 = debugger->vpu->stack[i]};
                fprintf(
                    debugger->output,
                    "%"PRIu64"- u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n",
                    i, v.as_uint64, v.as_int64, v.as_float64, v.as_ptr
                );
            }
            break;
        }
        EXPECT(argv[1], TKN_ULIT, stack_position);
        if(stack_position.value.as_uint64 >= debugger->vpu->registers[RSP >> 3].as_uint64){
            fprintf(debugger->output, "%"PRIu64" is out of stack's bounds (%"PRIu64")\n",
                stack_position.value.as_uint64, debugger->vpu->registers[RSP >> 3].as_uint64);
            return 1;
        } else{
            const Register v = (Register){.as_uint64 = debugger->vpu->stack[stack_position.value.as_uint64]};
            fprintf(debugger->output, "u: %"PRIu64", i: %"PRIi64", f: %f, p: %p\n", v.as_uint64, v.as_int64, v.as_float64, v.as_ptr);
        }
    }
        break;
    case DUPC_HELP:
        if(argc <= 1){
            fprintf(debugger->output, "help message...\n");
            fprintf(debugger->output, "press enter to step through the program\n");
            for(int i = 0; i < DUPC_DUPC_COUNT; i+=1){
                if(debugger_help(debugger, i)){
                    fprintf(debugger->err, "[ERROR] missing help message for dupc code %i\n", i);
                }
            }
        }
        else{
            for(int i = 1; i < argc; i+=1){
                const int dupc = get_dupc_code(argv[i]);
                if(dupc == DUPC_ERROR){
                    fprintf(debugger->output, "*** no command for %s\n", argv[i]);
                }
                else if(debugger_help(debugger, dupc)){
                    fprintf(debugger->err, "[ERROR] missing help message for dupc code %i\n", dupc);
                }
            }
        }
        break;
    default:
        fprintf(debugger->err, "[ERROR] Can't Perform User Prompt: Unknown UserPrompt Code (%i)\n", code);
        return 1;
    }

    return 0;
}

// \returns 0 on success, 1 if stdin was closed, or -1 on failure
int get_user_prompt(Debugger* debugger, int* _argc, char*** _argv){

    const uint64_t begin = debugger->stream.size;

    printf("\n(vpu) ");
    for(int c = fgetc(debugger->input); c && c != '\n'; ){
        char* inputstr = (char*) ((uintptr_t) debugger->stream.data);
        for(; debugger->stream.size < debugger->stream.capacity && c && c != '\n' && c != EOF; c = fgetc(stdin)){
            inputstr[debugger->stream.size++] = (char) c;
        }
        if(c == EOF){
            printf("\n");
            return 1;
        }
        // resize stream
        mc_stream(&debugger->stream, NULL, 0);
    }
    mc_stream_str(&debugger->stream, "");

    int biggest_word;
    const int word_count = get_word_count((char*) (((uintptr_t) debugger->stream.data) + begin), &biggest_word);

    if(word_count < 0) return -1;

    char** argv = (char**) mc_stream_aligned(&debugger->stream, NULL, word_count * sizeof(argv[0]), sizeof(argv[0]));

    char* promptstr = (char*) (((uintptr_t) debugger->stream.data) + begin);

    for(int i = 0; i < word_count; i+=1){
        argv[i] = promptstr;
        promptstr = get_next_word(promptstr, promptstr, NULL, biggest_word);
        promptstr += 1;
    }
    if(_argc) *_argc = word_count;
    if(_argv) *_argv = argv;

    return 0;
}

// debugs raw program and passes argc and argv to the executing program
// takes input from file at _input or stdin if input == NULL
// writes output to provided _output or stdout if output == NULL
// writes errors to _err or stderr if err == NULL
int debug(const char* input_file){

    VIRTUAL_DEBUG_LOG("requested debugging '%s'\n", input_file);

    if(!input_file){
        fprintf(stderr, "[ERROR] debug missing input file\n");
        return 1;
    }

    VirtualFile vfile;
    const char* required_fields[] = {
        VIRTUAL_FILE_PROGRAM_FIELD_NAME,
        NULL
    };
    const char* optional_fields[] = {
        VIRTUAL_FILE_LABELS_FIELD_NAME,
        VIRTUAL_FILE_STATIC_FIELD_NAME,
        NULL
    };
    if(vfopen(&vfile, input_file, required_fields, optional_fields)){
        fprintf(stderr, "[ERROR] debugger failed trying to open virtual file '%s'\n", input_file);
        return 1;
    }

    VIRTUAL_DEBUG_LOG("getting program...\n");
    const void* const program_field = get_virtual_file_field(vfile, VIRTUAL_FILE_PROGRAM_FIELD_NAME);
    uint64_t entry_point;
    uint64_t program_size;
    const Inst* program = get_program_from_vfield(program_field, &program_size, &entry_point);
    if(program == NULL){
        fprintf(stderr, "[ERROR] debugger failed, virtual file in '%s' has corrupt program\n", input_file);
        vfclose(vfile);
        return 1;
    }

    void* _labels = get_virtual_file_field(vfile, VIRTUAL_FILE_LABELS_FIELD_NAME);
    uint64_t labels_size = 0;
    if(_labels){
        labels_size = *(uint64_t*) _labels;
        labels_size -= sizeof(uint64_t) + sizeof(VIRTUAL_FILE_LABELS_FIELD_NAME);
        _labels = (void*) (((uintptr_t) _labels) + sizeof(uint64_t) + sizeof(VIRTUAL_FILE_LABELS_FIELD_NAME));
    }

    Mc_stream_t labels = (Mc_stream_t){.data = _labels, .size = labels_size, .capacity = 0, .alignment = 8};

    void* static_memory = get_virtual_file_field(vfile, VIRTUAL_FILE_STATIC_FIELD_NAME);
    uint64_t static_memory_size = 0;
    if(static_memory){
        static_memory_size = *(uint64_t*) static_memory;
        static_memory_size -= sizeof(uint64_t) + sizeof(VIRTUAL_FILE_STATIC_FIELD_NAME);
        static_memory = (void*) (((uintptr_t) static_memory) + sizeof(uint64_t) + sizeof(VIRTUAL_FILE_STATIC_FIELD_NAME));
    }

    VIRTUAL_DEBUG_LOG("setting up virtual processing unit\n");
    VPU vpu;
    vpu.program = (Inst*) program;
    vpu.static_memory = static_memory;

    Register registers[REGISTER_SPACE_SIZE / sizeof(Register)];
    for(int i = 0; i < REGISTER_SPACE_SIZE / sizeof(Register); i+=1){
        registers[i].as_uint64 = 0;
    }
    registers[RIP >> 3].as_uint64 = entry_point;
    vpu.register_space = (uint8_t*) registers;
    vpu.registers = (Register*) vpu.register_space;

    vpu.stack = (uint64_t*) malloc(1024);

    vpu.status = 0;

    VIRTUAL_DEBUG_LOG("setting up mini parser...\n");

    // for storing temporary data
    Mc_stream_t dstream = mc_create_stream(1024, 8);

    Tokenizer tokenizer = (Tokenizer){
        .data = (char*)((uint8_t*)(dstream.data) + sizeof(uint32_t) + *(uint32_t*)(dstream.data) + 1),
        .line = 0, .column = 0, .pos = 0
    };

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

    VIRTUAL_DEBUG_LOG("setting up debugger...\n");

    Debugger debugger = (Debugger){
        .input = stdin,
        .output = stdout,
        .err = stderr,
        .is_active = 1,
        .labels = labels,
        .parser = parser,
        .program = vpu.program,
        .program_size = program_size,
        .breakpoint_count = 0,
        .stream = dstream,
        .vpu = &vpu,
        .display_size = 5,
    };
    debugger.signals = malloc((size_t) debugger.program_size);
    memset(debugger.signals, 0, (size_t) debugger.program_size);

    VIRTUAL_DEBUG_LOG("start debugging...\n");

    debug_display_inst(&debugger, debugger.vpu->registers[RIP >> 3].as_uint64, debugger.display_size);

    int argc;
    char** argv;

    for(uint64_t scope = 0; debugger.is_active; debugger.stream.size = scope) {
	    
        scope = debugger.stream.size;

        const int sig = get_user_prompt(&debugger, &argc, &argv);

        // stdin closed
        if(sig == 1) break;
        if(sig < 0){
            fprintf(stderr, "[ERROR] invalid prompt\n");
            continue;
        }

        if(argc == 0){ // perform smooth step v v v
            if(debugger.vpu->registers[RIP >> 3].as_uint64 < debugger.program_size){
                VPU* const vpu = debugger.vpu;
                int ret_required_count = 0;
                if((debugger.program[debugger.vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL) do {
                    ret_required_count += ((debugger.program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL);
                    ret_required_count -= ((debugger.program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_RET);

                    vpu->registers[RIP >> 3].as_int64 += perform_inst(
                        vpu,
                        debugger.program[vpu->registers[RIP >> 3].as_uint64]
                    );

                } while(vpu->registers[RIP >> 3].as_uint64 < debugger.program_size && !vpu->status && ret_required_count > 0);
                else if(debugger.vpu->registers[RIP >> 3].as_uint64 < debugger.program_size){
                    debugger.vpu->registers[RIP >> 3].as_int64 += perform_inst(
                        debugger.vpu,
                        debugger.program[debugger.vpu->registers[RIP >> 3].as_uint64]
                    );
                }
            }                
            debug_display_inst(&debugger, debugger.vpu->registers[RIP >> 3].as_uint64, debugger.display_size);
            continue;
        }

        const int dupc = get_dupc_code(argv[0]);
        if(dupc == DUPC_ERROR){
            fprintf(debugger.err, "[ERROR] no command for %s, use help in case you want to check available commands\n", argv[0]);
        }
        else if(perform_user_prompt(&debugger, dupc, argc, argv)){
            fprintf(debugger.err, "*** command %s failed ^^^\n", argv[0]);
        }
    }


    VIRTUAL_DEBUG_LOG("finnished debugging, cleaning up...\n");
    free(vpu.stack);
    mc_destroy_stream(dstream);
    free(debugger.signals);
    vfclose(vfile);
    VIRTUAL_DEBUG_LOG("debugging complete\n");
    return vpu.status;
}



#endif // =====================  END OF FILE _VDEBUG_HEADER ===========================