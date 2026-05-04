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

    for(size_t i = 0; i < builder.iiis.size / sizeof(III); i+=1){
        print_iii(*(const III*) da_element(builder.iiis, i, III));
        printf("\n");
    }

    return 0;
}


