#include "tokenizer.h"
#include "parser.h"
#include "builder.h"
#include "../src/virtual_files.h"


int main(int argc, char** argv){

    int input_file_path_position = -1;
    const char* input_file_path = NULL;
    int output_file_path_position = -1;
    const char* output_file_path = "a.out";

    typedef enum {
        MODE_STD,
        MODE_TREE,
        MODE_INTERMEDIATE_REP
    } Mode;
    Mode mode = MODE_STD;

    for(int i = 1; i < argc; i+=1){

        if(!strcmp(argv[i], "--help")){
            printf(
                "compiles c code to the custom virtual machine byte code"
                "Usage %s [options] <input_file_path>\n"
                "available options are:\n"
                "  -i <input_file_path>:\n"
                "    takes <input_file_path> as the input_file, note that you can only provide one input file\n"
                "  -o <output_file_path>:\n"
                "    writes output (executable or other) to <output_file_path>\n"
                "  -t:\n"
                "    display parse tree\n"
                "  -r:\n"
                "    compile to intermediate representation\n"
                "  --help:\n"
                "    shows this help message\n",
                argv[0]
            );
            return 0;
        }
        if(!strcmp(argv[i], "-i")){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] expected input file path after %s flag\n", argv[i]);
                return 1;
            }
            if(input_file_path){
                fprintf(
                    stderr, "[ERROR] multiple input file path provided, '%s' in argument %i and '%s' in argument %i\n",
                    input_file_path, input_file_path_position, argv[i + 1], i + 1
                );
                return 1;
            }
            input_file_path = argv[++i];
            input_file_path_position = i;
            continue;
        }
        if(!strcmp(argv[i], "-o")){
            if(i + 1 >= argc){
                fprintf(stderr, "[ERROR] expected output file path after %s flag\n", argv[i]);
                return 1;
            }
            if(output_file_path_position > 0){
                fprintf(
                    stderr, "[ERROR] multiple output file path provided, '%s' in argument %i and '%s' in argument %i\n",
                    output_file_path, output_file_path_position, argv[i + 1], i + 1
                );
                return 1;
            }
            output_file_path = argv[++i];
            output_file_path_position = i;
            continue;
        }
        if(!strcmp(argv[i], "-t")){
            mode = MODE_TREE;
            continue;
        }
        if(!strcmp(argv[i], "-r")){
            mode = MODE_INTERMEDIATE_REP;
            continue;
        }

        if(input_file_path){
            fprintf(
                stderr, "[ERROR] multiple input file path provided, '%s' in argument %i and '%s' in argument %i\n",
                input_file_path, input_file_path_position, argv[i], i
            );
            return 1;
        }
        input_file_path = argv[i];
        input_file_path_position = i;
    }

    if(!input_file_path){
        fprintf(stderr, "Expected input file. Use --help to get a help message\n");
        return 1;
    }

    if(parse_file(input_file_path)){
        fprintf(stderr, "[ERROR] failed to parse file\n");
        return 1;
    }

    FILE* fout = NULL;
    if(mode == MODE_INTERMEDIATE_REP || mode == MODE_TREE){
        if(output_file_path_position < 0){
            fout = stdout;
        }
        else{
            fout = fopen(output_file_path, "wb");
        }
        if(!fout){
            fprintf(stderr, "[ERROR] could not open output file '%s'\n", output_file_path);
            return 1;
        }
    }
    
    
    #define MAIN_RET(STATUS) do { status = (STATUS); goto defer; } while(0)
    int status = 0;

    build_intermediate_interpretation();

    if(mode == MODE_TREE){
        if(display_tree(fout, 0, parser.statements.size / sizeof(Statement))){
            fprintf(stderr, "[ERROR] failed to display parse tree\n");
            MAIN_RET(1);
        }
        MAIN_RET(0);
    }
    if(mode == MODE_INTERMEDIATE_REP){
        for(size_t i = 0; i < da_len(builder.iii_index, int); i+=1){
            const int iiindex = *da_element(builder.iii_index, i, const int);
            const IIArg* const iia = da_element(builder.iias, iiindex, const IIArg);
            if(iia->type != IIATYPE_III){
                report_internal_error("iii of invalid type %i", iia->type);
                MAIN_RET(1);
            }
            print_iii(fout, da_element(builder.iias, iiindex, const IIArg)->value.iii);
            fputc('\n', fout);
        }
        MAIN_RET(0);
    }

    if(fout && fout != stdout){
        fclose(fout);
        fout = NULL;
    }

    if(build_assembly()){
        fprintf(stderr, "[ERROR] failed to build assembly\n");
        MAIN_RET(1);
    }

    VirtualFile vfile = create_virtual_file(
        output_file_path, EXEFLAG_NONE, VIRTUAL_FILE_TYPE_EXE,
        0, 0, NULL, NULL
    );
    if(add_virtual_file_field(&vfile, VIRTUAL_FILE_PROGRAM_FIELD_NAME, builder.assembly.size, builder.assembly.data)){
        fprintf(stderr, "[ERROR] failed to write assembly to virtual file\n");
        MAIN_RET(1);
    }
    if(vfsave(vfile, output_file_path)){
        fprintf(stderr, "[ERROR] failed to save virtual file to %s\n", output_file_path);
        MAIN_RET(1);
    }

defer:
    if(fout && fout != stdout)
        fclose(fout);

    return status;
}


