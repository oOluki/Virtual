#include "tokenizer.h"
#include "parser.h"



int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "Expected input file\n");
        return 1;
    }

    parse_file(argv[1]);

    return display_tree(0, parser.statements.size / sizeof(Statement));
}


