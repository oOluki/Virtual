#ifndef _VPU_ASSEMBLER
#define _VPU_ASSEMBLER

#include "virtual.h"
#include "virtual_files.h"
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

    Mc_stream_t files = mc_create_stream(1000, 0);

    if(!read_file_txt(&files, input_path, 1)){
        fprintf(stderr, "[ERROR] Could Not Open/Read File '%s'\n", input_path);
        mc_destroy_stream(files);
        return 1;
    }

    Mc_stream_t program = mc_create_stream(1024 * sizeof(Inst), 8);

    Mc_stream_t static_memory = mc_create_stream(0, 0);

    Mc_stream_t labels = mc_create_stream(0, 0);

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

    if(program.size % sizeof(Inst))
        VIRTUAL_DEBUG_WARN(
            "program stream does not have compatible size with Inst array, "
            "expected multiple of %i, got %"PRIu64" instead\n",
            (int) sizeof(Inst), program.size
        );

    mc_stream(&program, &parser.entry_point, sizeof(parser.entry_point));

    VirtualFile vfile = create_virtual_file(
        NULL,
        is_little_endian()? VIRTUAL_FILE_INTERNAL_FLAG_IS_LITTLE_ENDIAN : 0,
        VIRTUAL_FILE_TYPE_EXE, 0, 0, NULL, NULL
    );

    if(static_memory.size){
        add_virtual_file_field(&vfile, VIRTUAL_FILE_STATIC_FIELD_NAME, static_memory.size, static_memory.data);
    }
    if(labels.size){
        add_virtual_file_field(&vfile, VIRTUAL_FILE_LABELS_FIELD_NAME, labels.size, labels.data);
    }
    add_virtual_file_field(&vfile, VIRTUAL_FILE_PROGRAM_FIELD_NAME, program.size, program.data);

    if(vfsave(vfile, output_path? output_path : "output.out")){
        fprintf(stderr, "[ERROR] failed to save virtual file to '%s'\n", output_path? output_path : "output.out");
        status = 1;
    }

    defer:

    if(program.data)        mc_destroy_stream(program);
    if(static_memory.data)  mc_destroy_stream(static_memory);
    if(labels.data)         mc_destroy_stream(labels);
    if(files.data)          mc_destroy_stream(files);

    return status;
}

#endif // END OF FILE ============================================