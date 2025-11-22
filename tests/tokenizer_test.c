#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


typedef struct Tokenizer
{
    char*    data;
    uint64_t pos;
    int      line;
    int      column;
} Tokenizer;

typedef union TokenValue{
    char*    as_str;
    uint64_t as_uint;
    int64_t  as_int;
    double   as_float;
    char     as_char;
} TokenValue;

typedef struct Token
{
    TokenValue  value;
    int         size;
    //int         line;
    //int         column;
    uint8_t     type;
} Token;


enum TokenTypes{
    TKN_NONE = 0,
    TKN_RAW,
    TKN_INST,
    TKN_REG,
    TKN_NUM,
    TKN_ILIT,
    TKN_ULIT,
    TKN_FLIT,
    TKN_STR,
    TKN_CHAR,
    TKN_SPECIAL_SYM,
    TKN_MACRO_INST,
    TKN_LABEL_REF,
    TKN_EMPTY,
    TKN_ADDR_LABEL_REF,
    TKN_STATIC_SIZE,
    TKN_UNRESOLVED_LABEL,
    TKN_INST_POSITION,
    TKN_ERROR = 255,
};

typedef struct StringView
{
    char*    str;
    uint32_t size;
} StringView;

typedef struct LexizedString{
    char* str;
    int   read;
    int   written;
} LexizedString;


#define MKTKN(STR) ((Token){.value.as_str = STR, .size = sizeof(STR) - 1, .type = TKN_RAW})

#define is_char_numeric(CHARACTER) (get_digit(CHARACTER) >= 0)

int get_digit(char c){
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;

    default: return -1;
    }
}

char get_char_digit(int d){
    switch (d)
    {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';

    default: return '\0';
    }
}

uint8_t get_hex_digit(char c){
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;

    default: return 255;
    }
}

// returns the first found character position relative to the offset, or -1 if none are found
static inline int mc_find_char(const char* str, char c, int offset){
    str += offset;
    for(int i = 0; str[i]; i+=1){
        if(c == str[i]) return i;
    }
    return -1;
}

// works similar to cym_find_char but does so to any of the characters in the charbuff str
static inline int mc_find_chars(const char* str, const char* charbuff, int offset){
    str += offset;
    for(int i = 0; str[i]; i+=1){
        for(int j = 0; charbuff[j]; j+=1){
            if(charbuff[j] == str[i]) return i;
        }
    }
    
    return -1;
}


void fprint_token(FILE* file, const Token token){
    switch(token.type){
    case TKN_NONE:
	fprintf(file, "TOKEN_NONE");
	break;
    case TKN_RAW:
    case TKN_INST:
    case TKN_REG:
    case TKN_STR:
    case TKN_CHAR:
    case TKN_MACRO_INST:
    case TKN_LABEL_REF:
    case TKN_ADDR_LABEL_REF:
        fprintf(file, "%.*s", token.size, token.value.as_str);
        break;
    case TKN_ILIT:
        fprintf(file, "%"PRIi64"", token.value.as_int);
        break;
    case TKN_INST_POSITION:
    case TKN_ULIT:
        fprintf(file, "%"PRIu64"", token.value.as_uint);
        break;
    case TKN_FLIT:
        fprintf(file, "%f", token.value.as_float);
        break;
    case TKN_SPECIAL_SYM:
        fprintf(file, "%c", token.value.as_char);
        break;
    case TKN_EMPTY:
        fprintf(file, "TOKEN_EMPTY");
        break;
    case TKN_ERROR:
    default:
        fprintf(file, "TOKEN_ERROR(%"PRIu8")", token.type);
        break;
    }

}

static inline char get_escaped_char(char c){

    switch(c){

    case '0': return '\0';
    case 'n': return '\n';
    case 't': return '\t';
    default : return c;

    }

}

LexizedString lexize_str(char* str, char delim){
    int read_pos = 0;
    if(str[0] == delim) read_pos+=1;
    int write_pos = read_pos;
    
    while(str[read_pos] && (str[read_pos] != delim) && (str[read_pos] != '\n')){
	
	if(str[read_pos] == '\\'){
	    if(str[++read_pos] == '\0'){
		    return (LexizedString){.str = NULL, .read = read_pos, .written = write_pos};
	    }
	    int d = get_digit(str[read_pos]);
	    if((d < 8) && (d > -1)){
	        char c = 0;
	        for(int i = 0; (d < 8) && (i < 3) && (d > -1); i+=1){
                c = (c << 3) + d;
                d = get_digit(str[++read_pos]);
	        }
            str[write_pos++] = c;
            if(!str[read_pos] || (str[read_pos] == delim)) break;
            continue;
	    }
	    if(str[read_pos] == 'x'){
            uint8_t digit = get_hex_digit(str[++read_pos]);
            char c = digit;
            if(digit != 255){
                digit = get_hex_digit(str[++read_pos]);
                if(digit != 255){
                    c = (c << 4) | digit;
                    read_pos+=1;
                }
                str[write_pos++] = c;
                if(!str[read_pos] || (str[read_pos] == delim)) break;
            }
            else str[write_pos++] = 'x';
            continue;
	    }
	    
	    str[write_pos++] = get_escaped_char(str[read_pos++]);
	    continue;
	}
	str[write_pos++] = str[read_pos++];

    }
    return (LexizedString){
        .str = (str[read_pos] == delim)? str : NULL,
        .read = read_pos, .written = write_pos
    };
}


Token get_next_token(Tokenizer* tokenizer){
    char* string = tokenizer->data;
    const char* special_characters = ":,=";
    const char line_comment = ';';
    const char* delimiters = " \t\n\r;:,=";

    Token token = (Token){0};
    
    for(; string[tokenizer->pos]; tokenizer->pos+=1){
        while(string[tokenizer->pos] == ' ' || string[tokenizer->pos] == '\t'){
            tokenizer->column += 1;
            tokenizer->pos += 1;
        }
#ifdef _WIN32
        if(string[tokenizer->pos] == '\r'){
            if(string[tokenizer->pos + 1] == '\n'){
                tokenizer->line += 1;
                tokenizer->column = 0;
                tokenizer->pos += 1;
                continue;
            }
            tokenizer->column += 1;
            continue;
        }
#endif // END OF #ifdef _WIN32
        if(string[tokenizer->pos] == '\n'){
            tokenizer->line += 1;
            tokenizer->column = 0;
            continue;;
        }

        const char c = string[tokenizer->pos];
        if(c == '\"'){
            token.value.as_str = string + tokenizer->pos;
            const LexizedString ls = lexize_str(token.value.as_str + 1, '\"');
            tokenizer->column += ls.read + 2;
            tokenizer->pos += ls.read + 2;
            if(ls.str == NULL){
                token.type = TKN_ERROR;
                token.size = ls.written + 1;
                return token;
	        }
            token.value.as_str = ls.str - 1;
            token.size = ls.written + 2;
            token.value.as_str[token.size - 1] = '\"';
            token.type = TKN_STR;
            return token;
        }
	if(c == '\''){
	    token.value.as_str = string + tokenizer->pos;
	    const LexizedString ls = lexize_str(token.value.as_str + 1, '\'');
	    tokenizer->column += ls.read + 2;
	    tokenizer->pos += ls.read + 2;
	    if((ls.str == NULL) || (ls.written != 1)){
            token.type = TKN_ERROR;
            token.size = ls.written + 1;
            return token;
	    }
            token.value.as_str = ls.str - 1;
            token.size = ls.written + 2;
            token.value.as_str[token.size - 1] = '\'';
            token.type = TKN_CHAR;
            return token;
        }
        if(c == line_comment){
            int skip = mc_find_char(string + tokenizer->pos + 1, '\n', 0);
            if(skip < 0){
                return (Token){.value.as_str = NULL, .size = 0, .type = TKN_NONE};
            }
            tokenizer->pos += skip;
            tokenizer->column += skip + 1;
            continue;            
        }
        if(mc_find_char(special_characters, string[tokenizer->pos], 0) >= 0){
            token.value.as_char = string[tokenizer->pos];
            token.size = 1;
            token.type = TKN_SPECIAL_SYM;
            tokenizer->pos += 1;
            tokenizer->column += 1;
            return token;
        }
        token.size = mc_find_chars(string + tokenizer->pos, delimiters, 0);
        if(token.size < 0 && string[tokenizer->pos]){
            for(token.size = 0; string[token.size + tokenizer->pos]; token.size+=1);
        } else if(token.size < 0){
            break;
        }
        token.value.as_str = string + tokenizer->pos;
        switch (token.value.as_str[0])
        {
        case '\0':
            token.type = TKN_NONE;
            return token;
        case '%':
            token.type = TKN_MACRO_INST;
            break;
        case '$':
            token.type = TKN_LABEL_REF;
            break;
	    case '@':
	        token.type = TKN_ADDR_LABEL_REF;
	        break;
        
        default:
            token.type = TKN_RAW;
            break;
        }
        tokenizer->pos += token.size;
        tokenizer->column += token.size;
        return token;
    }
    token.type = TKN_NONE;
    return token;
}

char* read_file(const char* path){

    FILE* file = fopen(path, "r");
    if(!file) return NULL;

    if(fseek(file, 0, SEEK_END)){
        fclose(file);
        return NULL;
    }

    const long size = ftell(file);

    if(fseek(file, 0, SEEK_SET)){
        fclose(file);
        return NULL;
    }

    if(size < 0){
        fclose(file);
        return NULL;
    }

    void* data = malloc(size + 1);

    const size_t read = fread(data, 1, size, file);

    if(read != size){
        free(data);
        data = NULL;
    }
    else ((char*)data)[read] = '\0';

    fclose(file);

    return data;
}

int main(int argc, char** argv){


    if(argc != 2){
        fprintf(stderr, "[ERROR] %s expects exaclty one argument, file path, got %i instead\n", argv[0], argc - 1);
        return 1;
    }

    char* str = read_file(argv[1]);

    if(!str){
        fprintf(stderr, "[ERROR] could not read '%s'\n", argv[1]);
        return 1;
    }

    Tokenizer tokenizer = (Tokenizer){
        .data = str,
        .pos = 0,
        .line = 0,
        .column = 0
    };

    for(Token token = get_next_token(&tokenizer); token.type != TKN_NONE && token.type != TKN_ERROR; token = get_next_token(&tokenizer)){
        fprint_token(stdout, token);
        printf("\n");
    }


    free(str);

    return 0;
}


