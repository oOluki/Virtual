#define MC_INITMACRO_FORCE_64BIT 1
#include <stdio.h>
#include "parser.h"



int write_exe(Mc_stream_t* stream, const char* path, uint64_t entry_point, void* meta_data, uint64_t meta_data_size, uint32_t flags){

    int errstatus = 0;

    FILE* file = fopen(path, "wb");

    errstatus |= (!file);

    errstatus |= (fwrite("VPU", 4, 1, file) != 1);

    errstatus |= (fwrite(&flags, 4, 1, file) != 1);

    errstatus |= (fwrite(&meta_data_size, 8, 1, file) != 1);

    errstatus |= meta_data_size && (fwrite(meta_data, meta_data_size, 1, file) != 1);

    errstatus |= (fwrite(&entry_point, 8, 1, file) != 1);

    errstatus |= (fwrite(stream->data, 1, stream->size, file) != stream->size);

    if(((char*)stream->data)[stream->size - 1] != INST_HALT)
        errstatus |= (fputc(INST_HALT, file) != INST_HALT);

    fclose(file);

    if(errstatus)
        fprintf(stderr, "[ERROR] Could Not Write Executable To '%s'\n", path);

    return errstatus;
}

static inline Token token_from_cstr(char* cstr){
    Token token;
    token.value.as_str = cstr;
    for(token.size = 0; cstr[token.size]; token.size+=1);
    return token;
}



int main(int argc, char** argv){

    // X==X (DEBUG) X==X
    // argc = 2;
    // argv = malloc(argc * sizeof(char*));
    // argv[1] = "../examples/vstd_test.txt";

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

    Mc_stream_t files = (Mc_stream_t){.data = malloc(1000), .size = 0, .capacity = 1000};

    #ifdef _WIN32 // changing file separator to default '/'

        for(int i = 0; argv[input_file][i]; i+=1){
            if(argv[input_file][i] == '\\') argv[input_file][i] = '/';
        }

    #endif

    if(!read_file(&files, argv[input_file], "r", 1)){
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
    parser.tokenizer = &tokenizer;
    parser.entry_point = 0;
    parser.flags = FLAG_NONE;
    parser.macro_if_depth = 0;
    
    int status = parse_file(&parser, &program, &static_memory, &files);
    if(status) goto defer;

    *(uint64_t*)(static_memory.data) = static_memory.size;

    status = write_exe(&program, (output_file > 0)? argv[output_file] : "output.bin", parser.entry_point, static_memory.data, static_memory.size, EXE_DEFAULT);
    if(status) goto defer;

    defer:
    mc_destroy_stream(program);
    mc_destroy_stream(static_memory);
    mc_destroy_stream(labels);
    mc_destroy_stream(files);

    return status;
}

