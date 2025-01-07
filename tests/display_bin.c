#include <stdio.h>
#include <stdint.h>
#include <ctype.h>


static inline void pad(const int line_size_max_width){
    for(int i = 0; i < line_size_max_width; i+=1){
        printf(" ");
    }
}

int main(int argc, char** argv){

    if(argc != 2){
        fprintf(stderr, "Usage ./display_bin <binary_file>\n");
        return 1;
    }


    FILE* file = fopen(argv[1], "rb");
    if(!file){
        fprintf(stderr, "Could Not Open/Read File '%s'\n", argv[1]);
        return 1;
    }

    uint8_t buffer[10];
    if(fseek(file, 0, SEEK_END)){
        fprintf(file, "[ERROR] Could Not Seek File's End\n");
        return 1;
    }
    long f_size = ftell(file);
    if(fseek(file, 0, 0)){
        fprintf(file, "[ERROR] Could Not Seek File's Begin\n");
        return 1;
    }
    int line = 0;
    int padding = 0;
    for(int i = 1 + f_size / 10; i; i/=10) padding += 1;
    pad(padding);
    printf("  0 1 2 3  4 5 6 7  8 9    0123456789\n");

    while (!feof(file))
    {
        for(int l = 0; (l < 10) && !feof(file); l+=1){
            const int count = fread(buffer, 1, 10, file);
            printf("%i-", line++);
            pad(padding);

            int i = 0;
            for(; i < count && i < 4; i+=1){
                printf("%02x", buffer[i]);
            } printf(" ");
            for(; i < count && i < 8; i+=1){
                printf("%02x", buffer[i]);
            } printf(" ");
            for(; i < count; i += 1){
                printf("%02x", buffer[i]);
            } printf("   ");
            for(; i < 10; i+=1){
                printf("  ");
            }
            for(i = 0; i < count; i+=1){
                printf("%c", isprint(buffer[i])? (char) buffer[i] : '.');
            }
            printf("\n");
        }
        padding -= 1;

    }
    


    fclose(file);

    return 0;
}






