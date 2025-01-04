#include <stdio.h>


int main(int argc, char** argv){

    if(argc < 3 || argc > 4){
	printf("Usage ./cmpf <file1> <file2> <optional: modes>\n");
	return 1;
    }
    const char* modes = (argc == 4)? argv[3] : "r";
    FILE* f1 = fopen(argv[1], modes);
    FILE* f2 = fopen(argv[2], modes);
    int line = 0;
    int column = 0;
    int eq = 1;
    char c1;
    char c2;
    while(eq && !feof(f1) && !feof(f2)){
	c1 = fgetc(f1);
	c2 = fgetc(f2);
	if(c1 == '\n'){
	    line+=1;
	    column=0;
	} else column += 1;


	eq = c1 == c2;
    }

    if(!eq || (feof(f1) ^ feof(f2))){
	printf("files differ in line %i column %i\n", line, column);
	printf("%c != %c\n", c1, c2);
	return 1;
    }

    return 0;
}




