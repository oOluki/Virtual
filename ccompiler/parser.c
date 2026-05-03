#ifndef PARSER_C
#define PARSER_C

#include "parser.h"

Parser parser;

static inline const char* get_stmtkind_str(int stmtkind){
    switch (stmtkind)
    {
    case STMT_NONE:     return "STMT_NONE";
    case STMT_EXPR:     return "STMT_EXPR";
    case STMT_COMP:     return "STMT_COMP";
    case STMT_SELECT:   return "STMT_SELECT";
    case STMT_ITER:     return "STMT_ITER";
    case STMT_JMP:      return "STMT_JMP";
    default:
        report_internal_error("statement kind %i not implemented", stmtkind);
        return NULL;
    }
}

static inline const char* get_exprkind_str(int exprkind){
    switch (exprkind)
    {
    case EXPRTYPE_NONE:         return "EXPRTYPE_NONE";
    case EXPRTYPE_PRIMARY:      return "EXPRTYPE_PRIMARY";
    case EXPRTYPE_POSTFIX:      return "EXPRTYPE_POSTFIX";
    case EXPRTYPE_UNARY:        return "EXPRTYPE_UNARY";
    case EXPRTYPE_BINARY:       return "EXPRTYPE_BINARY";
    case EXPRTYPE_CONDITIONAL:  return "EXPRTYPE_CONDITIONAL";
    default:
        report_internal_error("expression kind %i not implemented", exprkind);
        return NULL;
    }
}

static int display_bin_expression(const BinExpr expr, int identation){
    TODO("display_bin_expression");
    return 0;
}

static int display_expression(int expr, int indentation){
    const Expr* const e = da_element(&parser.expressions, expr, Expr);
    printf("%*c%s:\n", indentation, '\t', get_exprkind_str(e->kind));
    switch (e->kind)
    {
    case EXPRTYPE_NONE:
        break;
    case EXPRTYPE_PRIMARY:
        printf("%*c", indentation + 1, '\t');
        print_token((Token){.type = e->expr.primary.kind, .value = e->expr.primary.value});
        break;
    case EXPRTYPE_POSTFIX:
        TODO("EXPRTYPE_POSTFIX");
        break;
    case EXPRTYPE_UNARY:
        TODO("EXPRTYPE_POSTFIX");
        break;
    case EXPRTYPE_BINARY:
        printf("%s\n", get_tkntype_str(e->expr.bin.op));
        display_expression(e->expr.bin.left, indentation + 1);
        display_expression(e->expr.bin.right, indentation + 1);
        break;
    case EXPRTYPE_CONDITIONAL:
        TODO("EXPRTYPE_CONDITIONAL");
        break;
    
    default:
        report_internal_error("expression kind %i not implemented", e->kind);
        return 1;
    }
}

static int display_stmt(int stmt, int indentation){
    const Statement* s = da_element(&parser.statements, stmt, Statement);
    printf("%*c%s:\n", indentation, '\t', get_stmtkind_str(s->kind));
    switch (s->kind)
    {
    case STMT_NONE:
        break;
    case STMT_EXPR:
        display_bin_expression(s->stmt.expr, indentation + 1);
        break;
    case STMT_COMP:
        display_stmt(s->stmt.comp, indentation + 1);
        break;
    case STMT_SELECT:
        //display_select_stmt(s->stmt.comp, indentation + 1);
        TODO("STMT_SELECT");
        break;
    case STMT_ITER:
        //display_iter_stmt(s->stmt.comp, indentation + 1);
        TODO("STMT_ITER");
        break;
    case STMT_JMP:
        //display_jmp_stmt(s->stmt.comp, indentation + 1);
        TODO("STMT_JMP");
        break;
        
    default:
        report_internal_error("statement kind %i not implemented", s->kind);
        return 1;
    }
    
    return 0;
}

int display_tree(int start, int end){
    for(int i = start; i >= 0 && i < end; i++){
        display_stmt(i, 0);
    }
    return 0;
}

int declare_symbol(const Symbol symbol){
    const int out = parser.symbols.size / sizeof(Symbol);
    DyArr* const da__ = &parser.symbols;
    da_append(da__, symbol, Symbol);
    return out;
}

int find_symbol(const Str name, int required){
    const DyArr* const da__ = &parser.symbols;
    if(da__->data) for(
        const Symbol* x = (const Symbol*) ((uintptr_t) da__->data + da__->size - sizeof(Symbol));
        (uintptr_t) x >= (uintptr_t) da__->data;
        x-=1){
        if(compare_str(x->name, name)){
            return (int) (((uintptr_t) (x) - (uintptr_t) (da__->data)) / sizeof(Symbol));
        }
    }
    if(required){
        report_error("could not find symbol '%.*s'", name.size, name.cstr);
    }
    return -1;
}

// print Intermediate Interpretation Argument
int print_iia(IIArg iia){
    switch (iia.type)
    {
    case IIATYPE_NONE:
        return 0;
    case IIATYPE_ULIT:
        printf("%u", iia.value.u);
        return 0;
    case IIATYPE_ILIT:
        printf("%i", iia.value.i);
        return 0;
    case IIATYPE_FLIT:
        printf("%f", iia.value.f);
        return 0;
    case IIATYPE_VAR:
        printf("%.*s", iia.value.str.size, iia.value.str.cstr);
        return 0;
    case IIATYPE_ERROR:
    default:
        printf("ERR");
        return 1;
    }
}

// print Intermediate Interpretation Instruction
int print_iii(III iii){
    int status = 0;
    switch (iii.iiiop)
    {
    case IIIOP_NONE:
        printf("NOP()");
        return 0;
    case IIIOP_RET:
        printf("RET(");
        status = print_iia(iii.arg[0]);
        printf(")");
        return status;
    case IIIOP_EXIT:
        printf("EXIT(");
        status = print_iia(iii.arg[0]);
        printf(")");
        return status;
    
    default:
        printf("ERROR(%i)", iii.iiiop);
        return 1;
    }
}


int get_keyword(Token token){

    if(token.type != TKNTYPE_RAW)
        return KEYW_NONE;

    if(comp_str_cstr(token.value.str, "const"))     return KEYW_CONST;
    if(comp_str_cstr(token.value.str, "static"))    return KEYW_STATIC;
    if(comp_str_cstr(token.value.str, "extern"))    return KEYW_EXTERN;
    if(comp_str_cstr(token.value.str, "void"))      return KEYW_VOID;
    if(comp_str_cstr(token.value.str, "char"))      return KEYW_CHAR;
    if(comp_str_cstr(token.value.str, "int"))       return KEYW_INT;
    if(comp_str_cstr(token.value.str, "float"))     return KEYW_FLOAT;
    if(comp_str_cstr(token.value.str, "double"))    return KEYW_DOUBLE;

    if(comp_str_cstr(token.value.str, "return"))    return KEYW_RETURN;
    if(comp_str_cstr(token.value.str, "goto"))      return KEYW_GOTO;
    if(comp_str_cstr(token.value.str, "break"))     return KEYW_BREAK;
    if(comp_str_cstr(token.value.str, "continue"))  return KEYW_CONTINUE;

    if(comp_str_cstr(token.value.str, "if"))        return KEYW_IF;
    if(comp_str_cstr(token.value.str, "else"))      return KEYW_ELSE;
    if(comp_str_cstr(token.value.str, "switch"))    return KEYW_SWITCH;

    if(comp_str_cstr(token.value.str, "for"))       return KEYW_FOR;
    if(comp_str_cstr(token.value.str, "while"))     return KEYW_WHILE;
    if(comp_str_cstr(token.value.str, "do"))        return KEYW_DO;

    return KEYW_NONE;
}

int keyword_type(int keyword){
    switch (keyword)
    {
    case KEYW_VOID:     return TYPE_VOID;
    case KEYW_CHAR:     return TYPE_CHAR;
    case KEYW_INT:      return TYPE_INT;
    case KEYW_FLOAT:    return TYPE_FLOAT;
    case KEYW_DOUBLE:   return TYPE_DOUBLE;
    
    default:    return TYPE_ERROR;
    }
}

static inline int decl_func(Str name, Variable ret, int argc, Variable* argv){

    return declare_symbol(
        (Symbol){
            .name = name,
            .type = TYPE_FUNC,
            .symbol.func = (Function){
                .argc = argc,
                .args = argv,
                .ret = ret,
                .body = 0
            }
        }
    );
}

int parse_file(const char* file){

    tokenizer.src = read_file(file).cstr;
    tokenizer.src_file_name = file;
    tokenizer.line = 1;

    //expect_cstr(tokenizer, "int");
    //expect_cstr(tokenizer, "main");
    //expect(tokenizer, '(');
    //expect(tokenizer, ')');
    //expect(tokenizer, '{');

    for(Token token = next_token(tokenizer); token.type != TKNTYPE_NONE; token = next_token(tokenizer)){

        const int keyword = get_keyword(token);
        if(keyword != KEYW_NONE){
            const int _type = keyword_type(keyword);
            if(_type != TYPE_ERROR){
                const Token name = expect(TKNTYPE_RAW);
                if(peek(0).type == '('){ // parse function arguments
                    skip(1);
                    expect(')');
                    const int f = decl_func(
                        name.value.str,
                        (Variable){
                            ._type = VARTYPE_BASIC,
                            .var.basic._type = _type
                        }, 0, NULL
                    );
                    
                    if(peek(0).type == '{'){
                        skip(1);
                        da_element(&parser.symbols, f, Symbol)->symbol.func.body = parse_compound_statement();
                    }
                    else{
                        expect(';');
                    }
                }
                else if(peek(0).type == '='){
                    const int lval = push_primary_expr(name);
                    skip(1);
                    const int rval = parse_rside_expression();
                    push_expr_stmt(lval, '=', rval);
                }
                else{
                    expect(';');
                }
            }
            else{
                report_error("declaration expected, got %s instead", get_tkntype_str(token.type));
            }
        }
        else if(token.type == TKNTYPE_RAW){
            parse_expression_statement(token);
        }
        else{
            report_error("invalid syntax%c", ' ');
        }


    }


    return 0;
}


#endif // =====================  END OF FILE PARSER_C ===========================