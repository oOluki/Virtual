#define MC_INITMACRO_FORCE_64BIT 1
#include <stdio.h>
#include "parser.h"
#include <inttypes.h>

int write_exe(Mc_stream_t* program, const char* path, uint64_t entry_point, void* meta_data, uint64_t meta_data_size, uint64_t flags){

    if(program->size % 4){
        fprintf(stderr, "[INTERNAL ERROR] " __FILE__ ":%i:9 : Program Size (%" PRIu64 ") Is Not Aligned To 4 Bytes\n"
        "Beware If You Are A User That This Is An Internal Error With The Compiler Implementation "
        "And Is Likelly Not Your Fault\n", (int) __LINE__, program->size);
        return 1;
    }

    uint8_t errstatus = 0;

    // padding bytes after the metadata to garantee the 4 byte alignment of the program pointer
    uint32_t padding = ((4 - (meta_data_size % 4)) % 4);

    FILE* file = fopen(path, "wb");

    if(!file){
	fprintf(stderr, "[ERROR] Could Not Write Executable To '%s', Unable To Open/Find File\n", path);
	return 1;
    }

    errstatus |= (fwrite("VPU:", 4, 1, file) != 1);

    errstatus |= (fwrite(&padding, 4, 1, file) != 1);

    errstatus |= (fwrite(&flags, 8, 1, file) != 1);

    errstatus |= (fwrite(&entry_point, 8, 1, file) != 1);

    errstatus |= (fwrite(&meta_data_size, 8, 1, file) != 1);

    uint64_t dummy = *(uint64_t*)meta_data;

    errstatus |= meta_data_size && (fwrite(meta_data, meta_data_size, 1, file) != 1);

    for( ; padding > 0; padding -= 1) errstatus |= (fwrite(&errstatus, 1, 1, file) != 1);

    errstatus |= (fwrite(program->data, 1, program->size, file) != program->size);

    fclose(file);

    if(errstatus)
        fprintf(stderr, "[ERROR] Could Not Write Valid Executable To '%s', EXECUTABLE GOT CORRUPTED DO NOT RUN IT\n", path);

    return errstatus;
}

static inline Token token_from_cstr(char* cstr){
    Token token;
    token.value.as_str = cstr;
    for(token.size = 0; cstr[token.size]; token.size+=1);
    return token;
}



int main(int argc, char** argv){

    #if 0 // X==X (DEBUG) X==X
    argc = 2;
    argv = malloc(argc * sizeof(char*));
    argv[1] = "../examples/hello_world.txt";
    #endif
    const Operand op = parse_op_literal((Token){.value.as_str = "10.0", .type = TKN_RAW});
    if(op.type == TKN_ERROR){
        fprintf(stderr, "wtf\n");
    }
    printf("%f\n", op.value.as_float64);

    int input_file  = -1;
    int output_file = -1;

    for(int i = 1; i < argc; i++){
        if(mc_compare_str(argv[i], "-o", 0)){
            if(output_file > 0){
                fprintf(stderr, "[ERROR] Multiple Output Flags, Output Flags In Arguments %i And %i\n", output_file, i);
                return 1;
            }
            output_file = ((i + 1 < argc) * (i + 1)) - !(i + 1 < argc);
            i+=1;
            continue;
        }
        if(input_file > 0){
            fprintf(stderr, "[ERROR] Multiple Input Files, Input Files In Arguments %i And %i\n", input_file, i);
            return 1;
        }

        input_file = i;

    }

    if(input_file < 0){
        fprintf(stderr, "[ERROR] Missing Input File\n");
        return 1;
    }

    Mc_stream_t program = mc_create_stream(1000);

    Mc_stream_t static_memory = mc_create_stream(sizeof(uint64_t) + sizeof("STATIC:"));

    mc_stream(&static_memory, &static_memory.size, sizeof(uint64_t));

    mc_stream(&static_memory, "STATIC:", sizeof("STATIC:"));

    Mc_stream_t labels = mc_create_stream(1000);

    Mc_stream_t files = mc_create_stream(1000);

    #ifdef _WIN32 // changing file separator to default '/'

        for(int i = 0; argv[input_file][i]; i+=1){
            if(argv[input_file][i] == '\\') argv[input_file][i] = '/';
        }

    #endif

    if(!read_file(&files, argv[input_file], 0, 1)){
        fprintf(stderr, "[ERROR] Could Not Open/Read File '%s'\n", argv[input_file]);
        mc_destroy_stream(program);
        mc_destroy_stream(static_memory);
        mc_destroy_stream(labels);
        mc_destroy_stream(files);
        return 1;
    }

    Tokenizer tokenizer = (Tokenizer){
        .data = (char*)((uint8_t*)(files.data) + sizeof(uint32_t) + *(uint32_t*)(files.data) + 1),
        .line = 0, .column = 0, .pos = 0
    };

    Parser parser;
    parser.file_path = (char*)((uint8_t*)(files.data) + sizeof(uint32_t));
    parser.file_path_size = *(uint32_t*)(files.data);
    parser.labels = &labels;
    parser.static_memory = &static_memory;
    parser.program = &program;
    parser.tokenizer = &tokenizer;
    parser.entry_point = 0;
    parser.flags = FLAG_NONE;
    parser.macro_if_depth = 0;
    
    int status = parse_file(&parser, &files);
    if(status) goto defer;

    *(uint64_t*)(static_memory.data) = static_memory.size;

    status = write_exe(
        &program,
        (output_file > 0)? argv[output_file] : "output.bin",
        parser.entry_point,
        static_memory.data,
        static_memory.size,
        EXE_DEFAULT
    );
    if(status){
        goto defer;
    }

    defer:
    mc_destroy_stream(program);
    mc_destroy_stream(static_memory);
    mc_destroy_stream(labels);
    mc_destroy_stream(files);

    return status;
}

