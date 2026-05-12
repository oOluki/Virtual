#ifndef TOKENIZER_C
#define TOKENIZER_C

#include "begin.h"
#include "tokenizer.h"

Tokenizer tokenizer;

const char* get_type_str(int _type){
    switch (_type)
    {
    case TYPE_VOID:     return "TYPE_VOID";
    case TYPE_CHAR:     return "TYPE_CHAR";
    //case TYPE_UCHAR:    return "TYPE_UCHAR";
    case TYPE_SHORT:    return "TYPE_SHORT";
    case TYPE_INT:      return "TYPE_INT";
    case TYPE_LONG:     return "TYPE_LONG";
    //case TYPE_UINT:     return "TYPE_UINT";
    case TYPE_FLOAT:    return "TYPE_FLOAT";
    case TYPE_DOUBLE:   return "TYPE_DOUBLE";
    case TYPE_PTR:      return "TYPE_PTR";
    case TYPE_VAR:      return "TYPE_VAR";
    case TYPE_FUNC:     return "TYPE_FUNC";
    case TYPE_ERROR:    return "TYPE_ERROR";
    default:
        report_internal_error("type %i not implemented", _type);
        return NULL;
    }
}

Str read_file(const char* file){

    FILE* f = fopen(file, "rb");
    Str str;

    if(!f){
        fprintf(stderr, "[ERROR] failed to open '%s'\n", file);
        exit(EXIT_FAILURE);
    }

    if(fseek(f, 0, SEEK_END)){
        report_error(__FILE__ ":%i:56: failed to open '%s', failed to fseek", __LINE__, file);
    }
    long fsize = ftell(f);
    if(fsize < 0){
        report_error(__FILE__ ":%i:56: failed to open '%s', failed to ftell", __LINE__, file);
    }
    if(fseek(f, 0, SEEK_SET)){
        report_error(__FILE__ ":%i:56: failed to open '%s', failed to fseek", __LINE__, file);
    }
    char* const data = (char*) malloc(fsize);
    if(!data){
        report_error(__FILE__ ":%i:56: failed to open '%s', mallox failed to allocate %zu bytes", __LINE__, file, (size_t) fsize);
    }
    str.size = fread(data, 1, fsize, f);
    if(str.size != fsize){
        free(data);
        report_error(__FILE__ ":%i:56: failed to open '%s', failed to read data", __LINE__, file);
    }
    str.cstr = data;

    return str;
}

int compare_str(Str str1, Str str2){
    if(str1.size != str2.size) return 0;
    for(size_t i = 0; i < str1.size && i < str2.size; i+=1){
        if(str1.cstr[i] != str2.cstr[i]) return 0;
    }
    return 1;
}

int comp_str_cstr(const Str str, const char* cstr){
    size_t i = 0;
    for(; i < str.size && cstr[i]; i+=1){
        if(str.cstr[i] != cstr[i]) return 0;
    }
    return i == str.size && !cstr[i];
}

void* da_stream(DyArr* da, const void* X, size_t SIZE){
    if(da->size + SIZE >= da->capacity){
        void* old = da->data;
        if(da->capacity == 0) da->capacity = SIZE + da->size;
        else da->capacity *= 1 + (size_t)((SIZE + da->size) / da->capacity);
        da->data = malloc(da->capacity);
        memcpy(da->data, old, da->size);
        free(old);
    }
    da->size += SIZE;
    return memcpy(da->data, (X), SIZE);
}

void* da_back(DyArr* da, size_t size){
    if(size > da->size){
        da->size = 0;
        return da->data;
    }
    da->size -= size;
    return (void*) (((uint8_t*) da->data) + da->size);
}

void* da_get(const DyArr da, size_t index, int require){
    if(index >= da.size){
        if(require)
            report_internal_error("list index(%zu) out of range(%zu)", index, da.size);
        return NULL;
    }
    return (void*) ((uintptr_t) (da.data) + index);
}

int special_character(char c){
    switch (c)
    {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ',':
        case '.':
        case ':':
        case ';':
        case '>':
        case '<':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '~':
        case '&':
        case '|':
        case '!':
        case '%':
        case '#':
            return 1;
    
    default:
        return 0;
    }
}


const char* get_tkntype_str(int tkntype){
    switch (tkntype)
    {
        case TKNTYPE_NONE:      return "TKNTYPE_NONE";
        case TKNTYPE_CHAR:      return "TKNTYPE_CHAR";
        case TKNTYPE_UCHAR:     return "TKNTYPE_UCHAR";
        case TKNTYPE_INT:       return "TKNTYPE_INT";
        case TKNTYPE_UINT:      return "TKNTYPE_UINT";
        case TKNTYPE_FLOAT:     return "TKNTYPE_FLOAT";
        case TKNTYPE_DOUBLE:    return "TKNTYPE_DOUBLE";
        case TKNTYPE_PTR:       return "TKNTYPE_PTR";
        case TKNTYPE_STR:       return "TKNTYPE_STR";
        case TKNTYPE_RAW:       return "TKNTYPE_RAW";

        case '(':   return "(";
        case ')':   return ")";
        case '[':   return "[";
        case ']':   return "]";
        case '{':   return "{";
        case '}':   return "}";
        case ',':   return ",";
        case '.':   return ".";
        case ':':   return ":";
        case ';':   return ";";
        case '>':   return ">";
        case '<':   return "<";
        case '=':   return "=";
        case '+':   return "+";
        case '-':   return "-";
        case '*':   return "*";
        case '/':   return "/";
        case '^':   return "^";
        case '~':   return "~";
        case '&':   return "&";
        case '|':   return "|";
        case '!':   return "!";
        case '#':   return "#";
        case '%':   return "%";
        case TKNTYPE_SR:  return ">>";
        case TKNTYPE_SL:  return "<<";
        case TKNTYPE_EE:  return "==";
        case TKNTYPE_OO:  return "||";
        case TKNTYPE_AA:  return "&&";
        case TKNTYPE_PP:  return "++";
        case TKNTYPE_MM:  return "--";
        case TKNTYPE_DE:  return "/=";
        case TKNTYPE_XE:  return "^=";
        case TKNTYPE_NE:  return "!=";
        case TKNTYPE_AE:  return "&=";
        case TKNTYPE_OE:  return "|=";
        case TKNTYPE_SRE: return ">>=";
        case TKNTYPE_SLE: return "<<=";
        case TKNTYPE_AAE: return "&&=";
        case TKNTYPE_OOE: return "||=";
        case TKNTYPE_FLIPE:  return "~=";
    
    default:
        return "TKNTYPE_ERROR";
    }
}

static int can_token_concat(int tkntype, int* cat1, int* cat2){
    if(cat1) *cat1 = TKNTYPE_ERROR;
    if(cat2) *cat2 = TKNTYPE_ERROR;

    switch (tkntype)
    {
    case '>':
    case '<':
    case '+':
    case '-':
    case '&':
    case '|':{
        if(cat2) *cat2 = tkntype;
    }
    case '*':
    case '/':
    case '^':
    case '~':
    case TKNTYPE_SR:
    case TKNTYPE_SL:{
        if(cat1) *cat1 = '=';
    }
        return 1;  
    default:
        return 0;
    }
}

int is_token_literal(int tkntype){
    switch (tkntype){
    case TKNTYPE_NONE:      return 1;
    case TKNTYPE_CHAR:      return 1;
    case TKNTYPE_UCHAR:     return 1;
    case TKNTYPE_INT:       return 1;
    case TKNTYPE_UINT:      return 1;
    case TKNTYPE_FLOAT:     return 1;
    case TKNTYPE_DOUBLE:    return 1;
    case TKNTYPE_PTR:       return 1;
    case TKNTYPE_STR:       return 1;
    case TKNTYPE_RAW:       return 1;
    default:
        return 0;
    }
}

int is_token_operand(int tkntype){
    switch (tkntype)
    {
        case '>':
        case '<':
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '~':
        case '&':
        case '|':
        case '!':
        case '%':
        case TKNTYPE_SR:
        case TKNTYPE_SL:
        case TKNTYPE_EE:
        case TKNTYPE_OO:
        case TKNTYPE_AA:
        case TKNTYPE_PP:
        case TKNTYPE_MM:
        case TKNTYPE_DE:
        case TKNTYPE_XE:
        case TKNTYPE_NE:
        case TKNTYPE_AE:
        case TKNTYPE_OE:
        case TKNTYPE_SRE:
        case TKNTYPE_SLE:
        case TKNTYPE_AAE:
        case TKNTYPE_OOE:
        case TKNTYPE_FLIPE:
            return tkntype;
    
    default:
        return 0;
    }
}

int is_token_assign(int tkntype){
    switch (tkntype)
    {
        case '=':
        case TKNTYPE_SR:
        case TKNTYPE_SL:
        case TKNTYPE_DE:
        case TKNTYPE_XE:
        case TKNTYPE_NE:
        case TKNTYPE_FLIPE:
        case TKNTYPE_AE:
        case TKNTYPE_OE:
        case TKNTYPE_SRE:
        case TKNTYPE_SLE:
        case TKNTYPE_AAE:
        case TKNTYPE_OOE:
            return tkntype;
    
    default:
        return 0;
    }
}

int toncat_operand(int op1, int op2){
    switch (op1)
    {
        case '=':  if(op2 == '=') return TKNTYPE_EE; else return 0;
        case '*':  if(op2 == '=') return TKNTYPE_TE; else return 0;
        case '/':  if(op2 == '=') return TKNTYPE_DE; else return 0;
        case '^':  if(op2 == '=') return TKNTYPE_XE; else return 0;
        case '!':  if(op2 == '=') return TKNTYPE_NE; else return 0;
        case '>':  if(op2 == '=') return TKNTYPE_GE; else if(op2 == op1) return TKNTYPE_SR; else return 0;
        case '<':  if(op2 == '=') return TKNTYPE_LE; else if(op2 == op1) return TKNTYPE_SL; else return 0;
        case '+':  if(op2 == '=') return TKNTYPE_PE; else if(op2 == op1) return TKNTYPE_PP; else return 0;
        case '-':  if(op2 == '=') return TKNTYPE_ME; else if(op2 == op1) return TKNTYPE_MM; else return 0;
        case '&':  if(op2 == '=') return TKNTYPE_AE; else if(op2 == op1) return TKNTYPE_AA; else return 0;
        case '|':  if(op2 == '=') return TKNTYPE_OE; else if(op2 == op1) return TKNTYPE_OO; else return 0;
        case '~':  if(op2 == '=')  return TKNTYPE_FLIPE;
        case TKNTYPE_SR: if(op2 == '=') return TKNTYPE_SRE;
        case TKNTYPE_SL: if(op2 == '=') return TKNTYPE_SLE;
        case TKNTYPE_OO: if(op2 == '=') return TKNTYPE_OOE;
        case TKNTYPE_AA: if(op2 == '=') return TKNTYPE_AAE;
    
    default:
        return 0;
    }
}

int print_token(const Token token){
    if(special_character(token.type))
        return printf("%c", token.type);
    switch (token.type)
    {
        case TKNTYPE_NONE:      return 0;
        case TKNTYPE_CHAR:      return printf("%c", token.value.c);
        case TKNTYPE_UCHAR:     return printf("%c", token.value.uc);
        case TKNTYPE_INT:       return printf("%i", token.value.i);
        case TKNTYPE_UINT:      return printf("%u", token.value.u);
        case TKNTYPE_FLOAT:     return printf("%f", token.value.f);
        case TKNTYPE_DOUBLE:    return printf("%f", token.value.lf);
        case TKNTYPE_PTR:       return printf("%p", token.value.p);
        case TKNTYPE_STR:       return printf("%s", token.value.cstr);
        case TKNTYPE_RAW:       return printf("%.*s", (int) token.value.str.size, token.value.str.cstr);
    
    default:
        return 1;
    }
}

Tokenizer create_tokenizer(const char* src, int pos, const char* src_file_name){
    return (Tokenizer){
        .src            = src,
        .src_file_name  = src_file_name,
        .pos            = 0,
        .line           = 1,
        .column         = 1,
        .tkn_buff_size  = 0
    };
}

int tokenize_cstr_numeric(TokenValue* value, const char* cstr, int* len){

    if(!cstr) return TKNTYPE_ERROR;

    int before_dot = 0;
    int after_dot  = 0;
    int dot        = 0;
    size_t i = 0;

    for(; cstr[i] <= '9' && cstr[i] >= '0'; i+=1)
    {
        before_dot = (before_dot * 10) + (cstr[i] - '0');
    }
    if(cstr[i] == '.'){
        dot = ++i;
    }
    for(; cstr[i] <= '9' && cstr[i] >= '0'; i+=1){
        after_dot = (after_dot * 10) + (cstr[i] - '0');
    }
    if(len) *len = i;
    if(dot){
        int decimal = 1;
        for(int j = dot; j < i; j+=1){
            decimal *= 10;
        }
        value->f = (double) (before_dot) + (double) (after_dot) / (double) (decimal);
        if(len && (cstr[i] == 'f' || cstr[i] == 'F' || cstr[i] == 'l' || cstr[i] == 'L'))
            *len += 1;
        return TKNTYPE_FLOAT;
    }

    value->u = before_dot;
    return TKNTYPE_UINT;
}


static int concat_token(int tkntype){
    int cat1;
    int cat2;
    int len = 0;
    while (can_token_concat(tkntype, &cat1, &cat2))
    {
        if(cat1 == TKNTYPE_NONE && cat2 == TKNTYPE_NONE)
            break;
        const int t1 = tokenizer.src[tokenizer.pos + len + 1];
        if(t1 == cat1 || t1 == cat2){
            tkntype = toncat_operand(tkntype, t1);
            len += 1;
        }
        else break;
    }
    tokenizer.pos += len;
    return tkntype;
}

int tokenize_cstr(TokenValue* value, const char* cstr, int* len){
    if(*cstr == '\0'){
        if(len) *len = 0;
        return TKNTYPE_NONE;
    }
    if(special_character(*cstr)){
        value->i = concat_token(*cstr);
        if(len) *len = 1;
        return value->i;
    }

    if(*cstr == '.' || (*cstr <= '9' && *cstr >= '0')){ // 
        return tokenize_cstr_numeric(value, cstr, len);
    }

    for(
        value->str = (Str){.cstr = cstr, .size = 0};
        cstr[value->str.size] != ' ' && cstr[value->str.size] != '\t' && cstr[value->str.size] != '\n' &&
        !special_character(cstr[value->str.size]);
        value->str.size += 1
    );

    if(len) *len = value->str.size;

    return TKNTYPE_RAW;
}

Token next_token(){
    if(tokenizer.tkn_buff_size > 0){
        const Token out = tokenizer.tkn_buff[0];
        for(int i = 1; i < tokenizer.tkn_buff_size; i+=1){
            tokenizer.tkn_buff[i - 1] = tokenizer.tkn_buff[i];
        }
        tokenizer.tkn_buff_size -= 1;
        tokenizer.line = out.line;
        tokenizer.column = out.column;
        tokenizer.src_file_name = out.file;
        return out;
    }

    const char* const src = tokenizer.src;
    for(char trimming = 1; trimming; ){
        for(
            ;
            src[tokenizer.pos] == ' ' || src[tokenizer.pos] == '\t' || src[tokenizer.pos] == '\n'
#ifdef _WIN32
            || src[tokenizer.pos] == '\r'
#endif // END OF #ifdef _WIN32
            ;
            tokenizer.pos += 1
        ){
            const char c = src[tokenizer.pos];
            const int column_skip = (src[tokenizer.pos] == '\t')? 4 : 1;
#ifdef _WIN32
            if(src[tokenizer.pos] == '\r'){
                tokenizer.column = 1;
                if(src[tokenizer.pos + 1] == '\n'){
                    tokenizer.line += 1;
                    tokenizer.pos += 1;
                    continue;
                }
                continue;
            }
#endif // END OF #ifdef _WIN32
            tokenizer.line   += c == '\n';
            tokenizer.column  = (c == '\n')? 1 : tokenizer.column + column_skip;
        }
        while(src[tokenizer.pos] == '/'){
            if(src[tokenizer.pos + 1] == '/'){
                for(; src[tokenizer.pos] != '\n' && src[tokenizer.pos] != '\0'; tokenizer.pos += 1){
#ifdef _WIN32
                    if(src[tokenizer.pos] == '\r'){
                        tokenizer.pos += 1;
                        tokenizer.column = 1;
                        if(src[tokenizer.pos] == '\n'){
                            tokenizer.line += 1;
                            tokenizer.pos += 1;
                            break;
                        }
                        continue;
                    }
#endif // END OF #ifdef _WIN32
                    tokenizer.column += 1;
                }
                if(src[tokenizer.pos] == '\n'){
                    tokenizer.column = 1;
                    tokenizer.line += 1;
                    tokenizer.pos += 1;
                }
            }
            else if(src[tokenizer.pos + 1] == '*'){/**/
                for(tokenizer.pos+=1; src[tokenizer.pos] != '\0'; tokenizer.pos += 1){
#ifdef _WIN32
                    if(src[tokenizer.pos] == '\r'){
                        tokenizer.pos += 1;
                        tokenizer.column = 1;
                        if(src[tokenizer.pos] == '\n'){
                            tokenizer.line += 1;
                            tokenizer.pos += 1;
                            continue;
                        }
                        continue;
                    }
#endif // END OF #ifdef _WIN32
                    if(src[tokenizer.pos] == '\n'){
                        tokenizer.column = 1;
                        tokenizer.line += 1;
                        tokenizer.pos += 1;
                        continue;
                    }
                    if(src[tokenizer.pos] == '*' && src[tokenizer.pos + 1] == '/'){
                        tokenizer.pos += 2;
                        tokenizer.column += 2;
                        break;
                    }
                }
            }
            else{
                break;
            }
        }
        if(src[tokenizer.pos] == ' ' || src[tokenizer.pos] == '\t' || src[tokenizer.pos] == '\n'
#ifdef _WIN32
            || src[tokenizer.pos] == '\r'
#endif // END OF #ifdef _WIN32
        )
            continue;
        if(src[tokenizer.pos] != '/') break;
        if(src[tokenizer.pos + 1] != '/' && src[tokenizer.pos + 1] != '*') break;
    }

    Token token = (Token){
        .file = tokenizer.src_file_name,
        .line = tokenizer.line,
        .column = tokenizer.column
    };

    int len = 0;
    token.type = tokenize_cstr(&token.value, tokenizer.src + tokenizer.pos, &len);
    if(token.type == TKNTYPE_ERROR){
        report_error("%s:%i:%i: invalid token", tokenizer.src_file_name, tokenizer.line, tokenizer.column);
    }
    tokenizer.pos += len;
    tokenizer.column += len;

    return token;
}

Token peek(int count){
    Tokenizer saved_tokenizer = tokenizer;
    int i = 0;
    Token token;
    for(; i < ARLEN(tokenizer.tkn_buff) && i < count && tokenizer.tkn_buff[i].type != TKNTYPE_NONE; i+=1){
        saved_tokenizer.tkn_buff[i] = next_token();
    }
    if(i == 0){
        token = next_token();
    }
    else for(token = tokenizer.tkn_buff[i - 1]; i < count && tokenizer.tkn_buff[i].type != TKNTYPE_NONE; i+=1){
        token = next_token();
    }

    tokenizer = saved_tokenizer;

    return token;
}

Token skip(int count){
    Token token;
    for(int i = 0; i < count; i+=1){
        token = next_token();
    }
    return token;
}

Token expect(int tkn_type){
    const Token token = next_token();
    if(token.type != tkn_type){
        report_error("expected %s, got %s instead", get_tkntype_str(tkn_type), get_tkntype_str(token.type));
    }
    return token;
}

Token expect_cstr(const char* cstr){
    const Token token = next_token();
    if(token.type != TKNTYPE_RAW){
        report_error("expected '%s' of type %s, got type %s instead", cstr, get_tkntype_str(TKNTYPE_STR), get_tkntype_str(token.type));
    }
    else if(0 == comp_str_cstr(token.value.str, cstr)){
        report_error("expected '%s', got %.*s instead", cstr, token.value.str.size, token.value.str.cstr);
    }
    return token;
}






#endif // =====================  END OF FILE TOKENIZER_C ===========================