#include "execute.c"
#include "disassembler.c"
#include "assembler.c"
#include "debugger.c"


int is_file_executable(FILE* file){

    if(!file) return 0;

    const long pos = ftell(file);

    char buff[5] = {'\0', '\0', '\0', '\0', '\0'};

    fread(buff, 1, sizeof(buff), file);

    fseek(file, pos, 0);

    return mc_compare_str(buff, "VPU:", 1);
}


static inline void help(const char* main_executable){
    printf(
        "Usage: %s [options] <input>\n"
        "Functionality: either assembles assembly program in <input> to byte code, disassembles byte code in <input> or executes byte code in <input>.\n"
        "Options:\n"
        "   --help:         displays this help message\n"
        "   --version:      displays VPU's current version\n"
        "   -assemble:      assemble mode\n"
        "   -disassemble:   disassemble mode\n"
        "   -execute:       execute mode\n"
        "   -debug:         debug mode\n"
        "   -o <output>:    choose <output> as output file\n"
        "   -i <input>:     choose <input> as input file\n"
        "   -args:          marks the beggining of the arguments to pass to executable\n"
        "   -export_labels: assembled executable/library will include all instruction position labels still defined by the end of the code\n",
        main_executable
    );
}


int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "[ERROR] Expected Input File, Use: %s --help to display help message\n", (argc == 1)? argv[0] : "?");
        return 1;
    }

    enum Mode{
        MODE_NONE = 0,
        MODE_ASSEMBLE    = 1 << 0,
        MODE_DISASSEMBLE = 1 << 1,
        MODE_EXECUTE     = 1 << 2,
        MODE_DEBUG       = 1 << 3
    } mode = MODE_NONE;

    int vpu_argv_begin  = argc;
    int input_file_arg  = -1;
    int output_file_arg = -1;
    int export_labels   = 0;
    
    for(int i = 1; i < argc; i++){
        if(mc_compare_str(argv[i], "--help", 0)){
            help(argv[0]);
            continue;
        }
        if(mc_compare_str(argv[i], "--version", 0)){
            printf(
                "vpu " VERSION "\n"
                "MIT License\n"
                "Copyright (c) 2024 oOluki\n"
                "WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY.\n"
            );
            continue;
        }
        if(mc_compare_str(argv[i], "-assemble", 0)){
            mode |= MODE_ASSEMBLE;
            continue;
        }
        if(mc_compare_str(argv[i], "-disassemble", 0)){
            mode |= MODE_DISASSEMBLE;
            continue;
        }
        if(mc_compare_str(argv[i], "-execute", 0)){
            mode |= MODE_EXECUTE;
            continue;
        }
        if(mc_compare_str(argv[i], "-debug", 0)){
            mode |= MODE_DEBUG;
            continue;
        }
        if(mc_compare_str(argv[i], "-o", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] Missing Filename After '-o'\n");
                return 1;
            }
            if(output_file_arg > 0){
                fprintf(stderr, "[ERROR] Multiple Output Flags, Output Flags In Arguments %i And %i\n", output_file_arg - 1, i);
                return 1;
            }
            output_file_arg = ++i;
            continue;
        }
        if(mc_compare_str(argv[i], "-i", 0)){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] Missing Filename After '-i'\n");
                return 1;
            }
            if(input_file_arg > 0){
                fprintf(stderr, "[ERROR] Multiple Input Files, Input Files In Arguments %i And %i\n", input_file_arg, i + 1);
                return 1;
            }
            input_file_arg = ++i;
            continue;
        }
        if(mc_compare_str(argv[i], "-export_labels", 0)){
            export_labels = 1;
            continue;
        }
        if(mc_compare_str(argv[i], "-args", 0)){
            vpu_argv_begin = i;
            break;
        }
        if(input_file_arg > 0){
            fprintf(stderr, "[ERROR] Multiple Input Files, Input Files In Arguments %i And %i\n", input_file_arg, i);
            return 1;
        }
    
        input_file_arg = i;
    
    }

    if(input_file_arg < 0){
        fprintf(stderr, "[ERROR] Missing Input File\n");
        return 1;
    }
    if((mode & MODE_DEBUG) && (mode & MODE_ASSEMBLE)){
        fprintf(stderr, "[ERROR] Can't Both Debug And Assemble A File\n");
        return 1;
    }
    if((mode & MODE_EXECUTE) && (mode & MODE_ASSEMBLE)){
        fprintf(stderr, "[ERROR] Can't Both Execute And Assemble A File\n");
        return 1;
    }
    if((mode & MODE_ASSEMBLE) && (mode & MODE_DISASSEMBLE)){
        fprintf(stderr, "[ERROR] Can't Both Assemble And Disassemble A File\n");
        return 1;
    }

    if(mode == MODE_NONE){
        FILE* input = fopen(argv[input_file_arg], "rb");
        if(!input){
            fprintf(stderr, "[ERROR] Can't open '%s'\n", argv[input_file_arg]);
            return 1;
        }
        mode = is_file_executable(input)? MODE_EXECUTE : MODE_ASSEMBLE;
        fclose(input);
    }

    if(mode & MODE_ASSEMBLE){
        const int status = assemble(argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : NULL, export_labels);
        if(status){
            fprintf(stderr, "[ERROR] Assembler Failed ^^^\n");
            return status;
        }
    }
    if(mode & MODE_DISASSEMBLE){
        const int status = disassemble(argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : NULL);
        if(status){
            fprintf(stderr, "[ERROR] Disassembler Failed ^^^\n");
            return status;
        }
    }

    if(mode & MODE_DEBUG){
        const int status = debug(argv[input_file_arg]);
        if(status){
            fprintf(stderr, "[ERROR] Debug Failed ^^^\n");
            return status;
        }
    }

    if(mode & MODE_EXECUTE){
        if(vpu_argv_begin > 1) argv[vpu_argv_begin - 2] = argv[0];
        if(vpu_argv_begin > 0) argv[vpu_argv_begin - 1] = argv[input_file_arg];
        const int status = execute(argv[input_file_arg], argc - vpu_argv_begin, argv + vpu_argv_begin);
        if(status){
            fprintf(stderr, "[ERROR] Execution Failed ^^^\n");
            return status;
        }
    }

    return 0;
}











