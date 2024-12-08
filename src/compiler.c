#define MC_INITMACRO_FORCE_64BIT 1
#include <stdio.h>
#include "parser.h"


int write_exe(Mc_stream_t* stream, const char* path, void* meta_data, uint64_t meta_data_size, uint32_t flags){

    int errstatus = 0;

    FILE* file = fopen(path, "wb");

    errstatus |= (!file);

    errstatus |= (fwrite("VPU", 4, 1, file) != 1);

    errstatus |= (fwrite(&flags, 4, 1, file) != 1);

    errstatus |= (fwrite(&meta_data_size, 8, 1, file) != 1);

    errstatus |= meta_data_size && (fwrite(meta_data, meta_data_size, 1, file) != 1);

    errstatus |= (fwrite(&entry_point, 8, 1, file) != 1);

    errstatus |= (fwrite(stream->data, 1, stream->size, file) != stream->size);

    if(stream->data[stream->size - 1] != INST_HALT)
        errstatus |= (fputc(INST_HALT, file) != INST_HALT);

    fclose(file);

    if(errstatus)
        fprintf(stderr, "[ERROR] Could Not Write Executable To '%s'\n", path);

    return errstatus;
}


int main(int argc, char** argv){

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

    //if(input_file < 0){
    //    fprintf(stderr, "[ERROR] Missing Input File\n");
    //    return 1;
    //}

    Mc_stream_t program = mc_create_stream(1000);

    Mc_stream_t static_memory = mc_create_stream(sizeof(uint64_t) + sizeof("STATIC:"));

    mc_stream(&static_memory, &static_memory.size, sizeof(uint64_t));

    mc_stream(&static_memory, "STATIC:", sizeof("STATIC:"));

    Mc_stream_t labels = mc_create_stream(1000);

    Mc_stream_t label_definitions = mc_create_stream(1000);

    flags = FLAG_EXPECT_INST;

    parse_file_recusive(&program, &static_memory, &labels, &label_definitions, (input_file > 0)? argv[input_file] : "../assembly/hello_world.vpu");

    //for(size_t i = 0; i < token_count; i += parse_instruction(&program, &static_memory, token_buffer, i, token_count));

    if(error_count){
        mc_destroy_stream(program);
        mc_destroy_stream(static_memory);
        mc_destroy_stream(labels);
        mc_destroy_stream(label_definitions);
        return 1;
    }

    *(uint64_t*)(static_memory.data) = static_memory.size;

    const int status = write_exe(&program, (output_file > 0)? argv[output_file] : "output.bin", static_memory.data, static_memory.size, EXE_DEFAULT);

    mc_destroy_stream(program);
    mc_destroy_stream(static_memory);
    mc_destroy_stream(labels);
    mc_destroy_stream(label_definitions);

    return status;
}

