#ifndef _VPU_ASSEMBLER
#define _VPU_ASSEMBLER

#include <stdio.h>
#include "parser.h"
#include <inttypes.h>




int write_exe(const Mc_stream_t* program, const char* path, uint64_t entry_point, const Mc_stream_t static_memory,
    const Mc_stream_t labels, uint64_t flags){

    if(program->size % 4){
        fprintf(stderr, "[INTERNAL ERROR] " __FILE__ ":%i:9 : Program Size (%" PRIu64 ") Is Not Aligned To 4 Bytes\n"
        "Beware If You Are A User That This Is An Internal Error With The Compiler Implementation "
        "And Is Likelly Not Your Fault\n", (int) __LINE__, program->size);
        return 1;
    }

    uint8_t errstatus = 0;

    const uint64_t meta_data_size = static_memory.size + labels.size + sizeof("LABELS:") + sizeof(labels.size);

    // padding bytes after the metadata to garantee the 4 byte alignment of the program pointer
    uint32_t padding = ((4 - (meta_data_size % 4)) % 4);

    const uint64_t labels_size = labels.size + sizeof("LABELS:") + sizeof(labels.size);

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

    errstatus |= static_memory.size && (fwrite(static_memory.data, (size_t) static_memory.size, 1, file) != 1);

    errstatus |= fwrite(&labels_size, sizeof(labels_size), 1, file) != 1;

    errstatus |= fwrite("LABELS:", sizeof("LABELS:"), 1, file) != 1;

    errstatus |= labels.size && (fwrite(labels.data, (size_t) labels.size, 1, file) != 1);

    for( ; padding > 0; padding -= 1) errstatus |= (fwrite(&errstatus, 1, 1, file) != 1);

    errstatus |= (fwrite(program->data, 1, program->size, file) != program->size);

    fclose(file);

    if(errstatus)
        fprintf(stderr, "[ERROR] Could Not Write Valid Executable To '%s', EXECUTABLE GOT CORRUPTED DO NOT RUN IT\n", path);

    return errstatus;
}

#ifdef _WIN32
// assembles program in input_path to output_path
int assemble(char* input_path, char* output_path, int export_labels){

    // changing file separator to default '/'
    for(size_t i = 0; input_path[i]; i+=1){
        if(input_path[i] == '\\') input_path[i] = '/';
    }
    for(size_t i = 0; output_path[i]; i+=1){
        if(output_path[i] == '\\') output_path[i] = '/';
    }

#else

// assembles program in input_path to output_path
int assemble(const char* input_path, const char* output_path, int export_labels){

#endif

    Mc_stream_t files = mc_create_stream(1000);

    if(!read_file(&files, input_path, 0, 1)){
        fprintf(stderr, "[ERROR] Could Not Open/Read File '%s'\n", input_path);
        mc_destroy_stream(files);
        return 1;
    }

    Mc_stream_t program = mc_create_stream(1000);

    Mc_stream_t static_memory = mc_create_stream(sizeof(uint64_t) + sizeof("STATIC:"));

    mc_stream(&static_memory, &static_memory.size, sizeof(uint64_t));

    mc_stream(&static_memory, "STATIC:", sizeof("STATIC:"));

    Mc_stream_t labels = mc_create_stream(1000);

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
    parser.flags = export_labels? EXEFLAG_LABELS_INCLUDED : EXEFLAG_NONE;
    parser.macro_if_depth = 0;
    
    int status = parse_file(&parser, &files);
    if(status) goto defer;

    *(uint64_t*)(static_memory.data) = static_memory.size;

    status = write_exe(
        &program,
        (output_path)? output_path : "output.bin",
        parser.entry_point,
        static_memory,
        labels,
        parser.flags
    );

    defer:
    mc_destroy_stream(program);
    mc_destroy_stream(static_memory);
    mc_destroy_stream(labels);
    mc_destroy_stream(files);

    return status;
}

#endif // END OF FILE ============================================