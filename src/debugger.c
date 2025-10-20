#ifndef _VDEBUG_HEADER
#define _VDEBUG_HEADER

#include "core.h"
#include "execute.c"
#include "parser.h"
#include "disassembler.c"

enum DebugUserPromptCode{
    DUPC_NONE  = 0,
    DUPC_CLEAR_VIEW,
    DUPC_EXIT,
    DUPC_GO,
    DUPC_GO_TILL_END,
    DUPC_PERFORM_INST,
    DUPC_ADD_BREAKPOINT,
    DUPC_REMOVE_BREAKPOINT,
    DUPC_STEP,
    DUPC_STEP_IN,
    DUPC_STEP_OUT,
    DUPC_DISREG,
    DUPC_DISPLAY_INST,
    DUPC_SHOW_LABEL,
    DUPC_SHOW_LABEL_AT,


    DUPC_ERROR = 255
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
    Mc_stream_t break_points;
    Mc_stream_t labels;

    Inst*       program;
    uint64_t    program_size;
} Debugger;

// \returns the index in memory where the breakpoint is, or -1 if there is no break point to x
int get_breakpoint_to(const Mc_stream_t break_points, uint64_t x){
    int i = 0;
    int f = (break_points.size)? break_points.size - 1 : 0;
    uint64_t lower = *(uint64_t*)mc_stream_on(&break_points, i);
    uint64_t upper = *(uint64_t*)mc_stream_on(&break_points, f);

    while (x != lower && x != upper && lower != upper)
    {
        const uint64_t n  = (i + f) / 2;
        const uint64_t nx = *(uint64_t*)mc_stream_on(&break_points, n);
        if(x <= nx){
            f     = n;
            upper = nx;
            continue;
        }
        i     = n;
        lower = nx;
    }
    if(x == lower) return i;
    if(x == upper) return f;
    return -1;
}

int get_prompt_code(const Token first_prompt_token){
    if(mc_compare_token(first_prompt_token, MKTKN("")       , 0))  return DUPC_CLEAR_VIEW;
    if(mc_compare_token(first_prompt_token, MKTKN("exit")   , 0))  return DUPC_EXIT;
    if(mc_compare_token(first_prompt_token, MKTKN("go")     , 0))  return DUPC_GO;
    if(mc_compare_token(first_prompt_token, MKTKN("goend")  , 0))  return DUPC_GO_TILL_END;
    if(mc_compare_token(first_prompt_token, MKTKN("break")  , 0))  return DUPC_ADD_BREAKPOINT;
    if(mc_compare_token(first_prompt_token, MKTKN("nobreak"), 0))  return DUPC_REMOVE_BREAKPOINT;
    if(mc_compare_token(first_prompt_token, MKTKN("step")   , 0))  return DUPC_STEP;
    if(mc_compare_token(first_prompt_token, MKTKN("stepin") , 0))  return DUPC_STEP_IN;
    if(mc_compare_token(first_prompt_token, MKTKN("stepout"), 0))  return DUPC_STEP_OUT;
    if(mc_compare_token(first_prompt_token, MKTKN("disreg") , 0))  return DUPC_DISREG;
    if(mc_compare_token(first_prompt_token, MKTKN("inst")   , 0))  return DUPC_DISPLAY_INST;
    if(mc_compare_token(first_prompt_token, MKTKN("label")  , 0))  return DUPC_SHOW_LABEL;
    if(mc_compare_token(first_prompt_token, MKTKN("labelat"), 0))  return DUPC_SHOW_LABEL_AT;

    return DUPC_ERROR;
}

int get_prompt_expected_argc(int code){
    switch (code)
    {
    case DUPC_NONE:
        return 0;
    case DUPC_CLEAR_VIEW:
        return 0;
    case DUPC_EXIT:
        return 0;
    case DUPC_GO:
        return 0;
    case DUPC_GO_TILL_END:
        return 0;
    case DUPC_PERFORM_INST:
        return 1;
    case DUPC_ADD_BREAKPOINT:
        return 1;
    case DUPC_REMOVE_BREAKPOINT:
        return 1;
    case DUPC_STEP:
        return 1;
    case DUPC_STEP_IN:
        return 0;
    case DUPC_STEP_OUT:
        return 0;
    case DUPC_DISREG:
        return 1;
    case DUPC_DISPLAY_INST:
        return 2;
    case DUPC_SHOW_LABEL:
        return 1;
    case DUPC_SHOW_LABEL_AT:
        return 1;
    default:
        return -1;
    }
}

int perform_user_prompt(Debugger* debugger, DebugUserPrompt prompt){
    switch (prompt.code)
    {
    case DUPC_NONE:
        break;
    case DUPC_CLEAR_VIEW:
        printf("\x1B[2J\x1B[H");
        break;
    case DUPC_EXIT:
        debugger->is_active = 0;
        break;
    case DUPC_GO:{
        for(int break_point = get_breakpoint_to(debugger->break_points, debugger->vpu->registers[RIP >> 3].as_uint64);
            debugger->vpu->registers[RIP >> 3].as_uint64 < debugger->program_size && !debugger->vpu->status && break_point < 0;
            break_point = get_breakpoint_to(debugger->break_points, debugger->vpu->registers[RIP >> 3].as_uint64)){

            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(
                debugger->vpu,
                debugger->program[debugger->vpu->registers[RIP >> 3].as_uint64]
            );
        }
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
        const Inst inst = prompt.arg1.as_uint32;
        if(inst == INST_JMP || inst == INST_JMPF || inst == INST_JMPFN || inst == INST_CALL || inst == INST_RET)
            debugger->vpu->registers[RIP >> 3].as_int64 += perform_inst(debugger->vpu, inst);
        else perform_inst(debugger->vpu, inst);
    }   break;
    case DUPC_ADD_BREAKPOINT:{
        uint64_t i = 0;
        uint64_t f = (debugger->break_points.size)? debugger->break_points.size - 1 : 0;
        uint64_t lower = *(uint64_t*)mc_stream_on(&debugger->break_points, i);
        uint64_t upper = *(uint64_t*)mc_stream_on(&debugger->break_points, f);
        const uint64_t x = prompt.arg1.as_uint64;
        if(x < lower){
            memmove(mc_stream_on(&debugger->break_points, sizeof(uint64_t)), debugger->break_points.data, sizeof(uint64_t));
            *(uint64_t*)mc_stream_on(&debugger->break_points, sizeof(uint64_t)) = x;
            return 0;
        }
        if(x > upper){
            mc_stream(&debugger->break_points, &x, sizeof(uint64_t));
            return 0;
        }
        while (x != lower && x != upper && lower != upper)
        {
            const uint64_t n  = (i + f) / 2;
            const uint64_t nx = *(uint64_t*)mc_stream_on(debugger->break_points.data, n);
            if(x <= nx){
                f     = n;
                upper = nx;
                continue;
            }
            i     = n;
            lower = nx;
        }
        // case where breakpoint already exists
        if(x == lower || x == upper) return 0;

        // reserve size of the breakpoint and allocate it on the appropriate index
        mc_stream(&debugger->break_points, &x, sizeof(uint64_t));
        memmove(mc_stream_on(&debugger->break_points, i + sizeof(uint64_t)), mc_stream_on(&debugger->break_points, i), sizeof(uint64_t));
        *(uint64_t*)mc_stream_on(&debugger->break_points, i) = x;
    }
        break;
    case DUPC_REMOVE_BREAKPOINT:
        if(prompt.arg1.as_uint64 * 8 >= debugger->break_points.size){
            fprintf(debugger->err, "[ERROR] Could Not Find Break Point To Remove\n");
            return 1;
        }
        memmove(
            mc_stream_on(&debugger->break_points, prompt.arg1.as_uint64 * 8),
            mc_stream_on(&debugger->break_points, prompt.arg1.as_uint64 * 8 + sizeof(uint64_t)),
            debugger->break_points.size - prompt.arg1.as_uint64 * 8 - sizeof(uint64_t)
        );
        break;
    case DUPC_STEP:{
        VPU* const vpu = debugger->vpu;
        const uint64_t next_ip = vpu->registers[RIP >> 3].as_uint64 + prompt.arg1.as_uint64;
        const uint64_t maxip   = ((debugger->program[vpu->registers[RIP >> 3].as_uint64] & 0xFF) == INST_CALL)?
            debugger->program_size : ((next_ip < debugger->program_size)? next_ip : debugger->program_size);

        while(vpu->registers[RIP >> 3].as_uint64 < maxip && !vpu->status &&
            vpu->registers[RIP >> 3].as_uint64 != next_ip){

            vpu->registers[RIP >> 3].as_int64 += perform_inst(
                vpu,
                debugger->program[vpu->registers[RIP >> 3].as_uint64]
            );
        }
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
    }
        break;
    case DUPC_DISREG:
        return 1;
    case DUPC_DISPLAY_INST:{
        VPU* const vpu = debugger->vpu;
        uint64_t i      = (prompt.arg2.as_uint64 < prompt.arg1.as_uint64)?
            prompt.arg1.as_uint64 - prompt.arg2.as_uint64 : 0;
        uint64_t finish = (prompt.arg1.as_uint64 + prompt.arg2.as_uint64 < debugger->program_size)?
            prompt.arg1.as_uint64 + prompt.arg2.as_uint64 : debugger->program_size;
        char charbuff[30];

        char* buff[3] = {
            &charbuff[0],
            &charbuff[10],
            &charbuff[20]
        };
        int status = 0;
        for(; i < finish && i < vpu->registers[RIP >> 3].as_uint64 && !status; i+=1){
            if(get_breakpoint_to(debugger->break_points, i) >= 0) fprintf(debugger->output, "!");
            status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
        }
        if(i == vpu->registers[RIP >> 3].as_uint64 && !status){
            fprintf(debugger->output, "*");
            if(i >= debugger->program_size){
                fputc('\n', debugger->output);
            } else{
                if(get_breakpoint_to(debugger->break_points, i) >= 0) fprintf(debugger->output, "!");
                status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i++, buff);
            }
        }
        for(; i < finish && !status; i+=1){
            if(get_breakpoint_to(debugger->break_points, i) >= 0) fprintf(debugger->output, "!");
            status = print_inst(debugger->output, debugger->program, debugger->vpu->static_memory, i, buff);
        }
        if(status){
            fprintf(debugger->err, "[ERROR] While Displaying Instruction %"PRIu32" At %"PRIu64"\n", debugger->program[i], i);
            return 1;
        }
    }
        break;
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
            "\tdefinition: ",
            (size_t)((uintptr_t)(label_ptr) - (uintptr_t)(debugger->parser.labels->data)), label.size,
            (int) prompt.arg2.as_int32, (char*) prompt.arg1.as_ptr
        );
        if(label.type == TKN_STR){
            fprintf(debugger->output, "%.*s\n", (int)(*(uint64_t*)((uint8_t*)(label_ptr) + label.definition.as_uint)),
                (char*)((uint8_t*)(label_ptr) + label.definition.as_uint));
        }
        else{
            fprintf(debugger->output, "u: %lu, i: %li, f: %f, p: %p\n", label.definition.as_uint, label.definition.as_int,
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
            "\tdefinition: ",
            (size_t)((uintptr_t)(label_ptr) - (uintptr_t)(debugger->parser.labels->data)), label.size,
            (int) label.str_size, (const char*) ((uint8_t*) (label_ptr) + label.str)
        );
        if(label.type == TKN_STR){
            fprintf(debugger->output, "%.*s\n", (int)(*(uint64_t*)((uint8_t*)(label_ptr) + label.definition.as_uint)),
                (char*)((uint8_t*)(label_ptr) + label.definition.as_uint));
        }
        else{
            fprintf(debugger->output, "u: %lu, i: %li, f: %f, p: %p\n", label.definition.as_uint, label.definition.as_int,
                label.definition.as_float, label.definition.as_str);
        }
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
        .pos  = 0, .data   = (uint8_t*) (debugger->stream.data) + begin
    };
    Token token = get_next_token(debugger->parser.tokenizer);
    const int code = get_prompt_code(token);
    if(code == DUPC_ERROR){
        const InstProfile inst_profile = get_inst_profile(token);
        if(inst_profile.opcode != INST_ERROR){
            const Inst inst = parse_inst(&debugger->parser, inst_profile, (StringView){.str = token.value.as_str, .size = token.size});
            if(inst == INST_CONTAINER){
                uint32_t container[2] = {
                    ((Inst*) debugger->parser.program->data)[(debugger->parser.program->size / sizeof(Inst)) - 1],
                };
                if(container[0] == INST_CONTAINER){ // LOAD2 case
                    container[1] = ((Inst*) debugger->parser.program->data)[(debugger->parser.program->size / sizeof(Inst)) - 2];
                    *(uint64_t*)(debugger->vpu->register_space + (container[1] & 0x0000FF00)) =
                        (uint64_t) ((uint64_t) (inst & 0XFFFFFF00) << 32) |
                        ((uint64_t) (container[0] & 0XFFFFFF00) << 8) | ((uint64_t) (container[1] & 0XFFFF0000) >> 16);
                    debugger->stream.size = begin;
                    return (DebugUserPrompt){.code = DUPC_NONE};
                } else{ // LOAD1 case
                    *(uint32_t*)(debugger->vpu->register_space + (container[0] & 0x0000FF00)) =
                        ((uint64_t) (inst & 0XFFFFFF00) << 8) | ((uint64_t) (container[0] & 0XFFFF0000) >> 16);
                    debugger->stream.size = begin;
                    return (DebugUserPrompt){.code = DUPC_NONE};
                }
            }
            debugger->stream.size = begin;
            if(inst == INST_ERROR)
                return (DebugUserPrompt){.code = DUPC_ERROR};
            return (DebugUserPrompt){.code = DUPC_PERFORM_INST, .arg1.as_uint32 = inst};
        }
        debugger->stream.size = begin;
        return (DebugUserPrompt){.code = DUPC_ERROR};
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
        argv[i].as_ptr = token.value.as_str;
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
    // for stroing temporary data
    Mc_stream_t dstream = mc_create_stream(1000);

    Parser parser;
    parser.file_path = "stdin";
    parser.file_path_size = sizeof("stdin") - sizeof("");
    parser.labels = &labels;
    parser.static_memory = NULL;
    parser.program = &dstream;
    parser.tokenizer = &tokenizer;
    parser.entry_point = 0;
    parser.flags = EXEFLAG_NONE;
    parser.macro_if_depth = 0;

    Debugger debugger;
    debugger.break_points = mc_create_stream(1000);
    debugger.input = stdin;
    debugger.output = stdout;
    debugger.err = stderr;
    debugger.is_active = 1;
    debugger.labels = labels;
    debugger.parser = parser;
    debugger.program = vpu.program;
    debugger.program_size = program_size;
    debugger.stream = dstream;
    debugger.vpu = &vpu;

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
    mc_destroy_stream(debugger.break_points);
    mc_destroy_stream(debugger.stream);

    return vpu.status;
}



#endif // =====================  END OF FILE _VDEBUG_HEADER ===========================