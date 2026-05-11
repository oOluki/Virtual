#include "tokenizer.h"
#include "parser.h"
#include "builder.h"


int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "Expected input file\n");
        return 1;
    }

    parse_file(argv[1]);

    display_tree(0, parser.statements.size / sizeof(Statement));

    interpret();

    printf("\nitermediate representation:\n");

    for(size_t i = 0; i < da_len(builder.iii_index, int); i+=1){
        const int iiindex = *da_element(builder.iii_index, i, const int);
        const IIArg* const iia = da_element(builder.iias, iiindex, const IIArg);
        if(iia->type != IIATYPE_III){
            report_internal_error("iii of invalid type %i", iia->type);
        }
        print_iii(da_element(builder.iias, iiindex, const IIArg)->value.iii);
        printf("\n");
    }

    return 0;
}


