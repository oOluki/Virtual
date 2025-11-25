#include "system.c"
#include "assembler.c"
#include "core.c"
#include "disassembler.c"
#include "debugger.c"


int is_file_executable(FILE* file){

    if(!file) return 0;

    const long pos = ftell(file);

    char buff[5] = {'\0', '\0', '\0', '\0', '\0'};

    fread(buff, 1, sizeof(buff), file);

    fseek(file, pos, 0);

    return mc_compare_str(buff, VIRTUAL_FILE_MAGIC_NUMBER, 1);
}


static inline void help(const char* main_executable){
    printf(
        "Usage: %s [options] <input>\n"
        "Functionality: either assembles assembly program in <input> to byte code, disassembles byte code in <input> or executes byte code in <input>.\n"
        "Options:\n"
        "   --help:         displays this help message\n"
        "   --version:      displays current version\n"
        "   --inst:         displays instructions desciptions and examples\n"
        "   -assemble:      assemble mode\n"
        "   -disassemble:   disassemble mode\n"
        "   -execute:       execute mode\n"
        "   -debug:         debug mode\n"
        "   -o <output>:    choose <output> as output file\n"
        "   -i <input>:     choose <input> as input file\n"
        "   -args:          marks the beggining of the arguments to pass to the virtual machine executable\n"
        "   -0:             pass no argument to the virtual machine executable\n"
        "   -no_export_labels: assembled executable/library will not include labels still defined by the end of the code\n",
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

    int vpu_argv_begin  = argc - 1;
    int input_file_arg  = -1;
    int output_file_arg = -1;
    int export_labels   = 1;

    VIRTUAL_DEBUG_LOG("parsing cmd arguments\n");
    
    for(int i = 1; i < argc; i++){
        if(mc_compare_str(argv[i], "--help", 0)){
            help(argv[0]);
            return 0;
        }
        if(mc_compare_str(argv[i], "--version", 0)){
            printf(
                "vpu " VERSION "\n"
                "MIT License\n"
                "Copyright (c) 2024 oOluki\n"
                "WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY.\n"
            );
            return 0;
        }
        if(mc_compare_str(argv[i], "--inst", 0)){
            char _buff[24];
            char* buff[] = {&_buff[0], &_buff[8], &_buff[16]};
            for(Inst i = 0; i < INST_TOTAL_COUNT; i+=1){
                if(print_inst_description(stdout, i)){
                    fprintf(stderr, "[ERROR] print_inst_description missing %"PRIu32"th instruction\n", i);
                    return 1;
                }
                printf("\texample: ");
                if(print_inst(stdout, i, buff)){
                    fprintf(stderr, "[ERROR] print_inst missing %"PRIu32"th instruction\n", i);
                    return 1;
                }
            }
            return 0;
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
        if(mc_compare_str(argv[i], "-no_export_labels", 0)){
            export_labels = 0;
            continue;
        }
        if(mc_compare_str(argv[i], "-args", 0)){
            if(vpu_argv_begin < 0){
                fprintf(stderr, "[ERROR] Can't use -args flag together with -0\n");
                return 1;
            }
            vpu_argv_begin = i;
            break;
        }
        if(mc_compare_str(argv[i], "-0", 0)){
            vpu_argv_begin = -1;
            continue;
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
        VIRTUAL_DEBUG_LOG("no mode provided, deducing best mode\n");
        FILE* input = fopen(argv[input_file_arg], "rb");
        if(!input){
            fprintf(stderr, "[ERROR] Can't open '%s'\n", argv[input_file_arg]);
            return 1;
        }
        mode = is_file_executable(input)? MODE_EXECUTE : MODE_ASSEMBLE;
        VIRTUAL_DEBUG_LOG("choose %s\n", (mode == MODE_EXECUTE)? "execute" : "assemble");
        fclose(input);
    }

    if(mode & MODE_ASSEMBLE){
        VIRTUAL_DEBUG_LOG("assembling %s to %s\n", argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : "output.out");
        const int status = assemble(argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : NULL, export_labels);
        if(status){
            fprintf(stderr, "[ERROR] Assembler Failed ^^^\n");
            return status;
        }
    }
    if(mode & MODE_DISASSEMBLE){
        VIRTUAL_DEBUG_LOG("disassembling %s to %s\n", argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : "stdout");
        const int status = disassemble_file(argv[input_file_arg], (output_file_arg > 0)? argv[output_file_arg] : NULL);
        if(status){
            fprintf(stderr, "[ERROR] Disassembler Failed ^^^\n");
            return status;
        }
    }

    else if((mode & MODE_EXECUTE) || (mode & MODE_DEBUG)){
        char* save_argvn1 = NULL;
        char* save_argv0 = NULL;
        const char* input_file_path = argv[input_file_arg];
        const int   program_argc =  (vpu_argv_begin > 0)? argc - vpu_argv_begin : 0;
        char** const program_argv = (vpu_argv_begin > 0)? argv + vpu_argv_begin : NULL;
        if(vpu_argv_begin > 0 ){
            save_argvn1 = argv[vpu_argv_begin - 1];
            argv[vpu_argv_begin - 1] = argv[0];
        }
        if(vpu_argv_begin > -1){
            save_argv0 = argv[vpu_argv_begin];
            argv[vpu_argv_begin] = argv[input_file_arg];
        }
        if(mode & MODE_EXECUTE){
            const int status = execute(input_file_path, program_argc, program_argv);
            if(status){
                fprintf(stderr, "[ERROR] Execution Failed ^^^\n");
                return status;
            }
        }
        else{
            const int status = debug(input_file_path, program_argc, program_argv);
            if(status){
                fprintf(stderr, "[ERROR] Debug Failed ^^^\n");
                return status;
            }
        }
        if(vpu_argv_begin > 0 ){
            argv[vpu_argv_begin - 1] = save_argvn1;
        }
        if(vpu_argv_begin > -1){
            argv[vpu_argv_begin] = save_argv0;
        }
    }

    return 0;
}











