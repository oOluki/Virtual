#ifndef PARSER_C
#define PARSER_C

#include "parser.h"

Parser parser;

static inline const char* get_stmtkind_str(int stmtkind){
    switch (stmtkind)
    {
    case STMT_NONE:     return "STMT_NONE";
    case STMT_DECL:     return "STMT_DECL";
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

static inline void indent(int indentation){
    for(int i = 0; i < indentation; i+=1)
        putchar(' ');
}

static int display_expression(int expr, int indentation);

static int display_bin_expression(const BinExpr expr, int indentation){
    printf("%*c%s\n", indentation, ' ', get_tkntype_str(expr.op));
    display_expression(expr.left, indentation);
    display_expression(expr.right, indentation);
    return 0;
}

static int display_expression(int expr, int indentation){
    const Expr* const e = da_element(parser.expressions, expr, Expr);
    indent(indentation);
    printf("%s:\n", get_exprkind_str(e->kind));
    switch (e->kind)
    {
    case EXPRTYPE_NONE:
        break;
    case EXPRTYPE_PRIMARY:
        indent(indentation + 2);
        print_token((Token){.type = e->expr.primary.kind, .value = e->expr.primary.value});
        printf("\n");
        break;
    case EXPRTYPE_POSTFIX:
        TODO("EXPRTYPE_POSTFIX");
        break;
    case EXPRTYPE_UNARY:
        TODO("EXPRTYPE_POSTFIX");
        break;
    case EXPRTYPE_BINARY:
        display_bin_expression(e->expr.bin, indentation + 2);
        break;
    case EXPRTYPE_CONDITIONAL:
        TODO("EXPRTYPE_CONDITIONAL");
        break;
    
    default:
        report_internal_error("expression kind %i not implemented", e->kind);
        return 1;
    }
    return 0;
}

static int display_sym_flags(int flags){
    if(flags & SYMFLAG_PLACEHOLDER)
        return 0;

    if(flags & SYMFLAG_STATIC)
        printf("static ");
    else if(flags & SYMFLAG_EXTERN)
        printf("extern ");
    
    if(flags & SYMFLAG_INLINE){
        printf("inline ");
    }

    if(flags & SYMFLAG_CONST){
        printf("const ");
    }
    else {
        printf("var ");
    }

    if(flags & SYMFLAG_UNSIGNED){
        printf("unsigned ");
    }

    return 0;
}

static int display_symvar(const Str name, const Variable* var){
    display_sym_flags(var->flags);
    printf(
        "%.*s%c %*c",
        name.size, name.cstr,
        (name.size > 0)? ':' : ' ',
        var->pointer_counter, (var->pointer_counter > 0)? '*' : ' '
    );

    switch (var->_type)
    {
    case VARTYPE_BASIC:
        printf("%s", get_type_str(var->var.basic._type));
        break;
    case VARTYPE_ARR:
        printf("%s[%i]", get_type_str(var->var.arr.base_type), var->var.arr.array_len);
        break;
    
    default:
        report_internal_error("variable type %i not implemented", var->_type);
        break;
    }
    return 0;
}

static int display_sym(const Symbol* sym, int indentation){
    indent(indentation);
    if(sym->_type == TYPE_VAR){
        display_symvar(sym->name, &sym->symbol.var);
        printf("\n");
    }
    else if(sym->_type == TYPE_FUNC){
        printf("func %.*s(", sym->name.size, sym->name.cstr);
        for(int i = 0; i < sym->symbol.func.argc; i+=1){
            display_symvar((Str){}, sym->symbol.func.args + 1);
            printf(", ");
        }
        printf(") -> "); display_symvar((Str){}, &sym->symbol.func.ret);
        printf("\n");
    }
    else{
        report_internal_error("symbol type %i not implemented", sym->_type);
    }
    return 0;
}

static int display_stmt(int stmt, int indentation){
    const Statement* s = da_element(parser.statements, stmt, Statement);
    indent(indentation);
    printf("%s:\n", get_stmtkind_str(s->kind));
    switch (s->kind)
    {
    case STMT_NONE:
        break;
    case STMT_DECL:
        display_sym(da_element(parser.symbols, s->stmt.decl, Symbol), indentation + 2);
        break;
    case STMT_EXPR:
        display_expression(s->stmt.expr, indentation + 2);
        break;
    case STMT_COMP:
        indent(indentation + 2);
        printf("block(%i, %i)\n", s->stmt.comp.start, s->stmt.comp.end);
        break;
    case STMT_SELECT:
        //display_select_stmt(s->stmt.comp, indentation + 1);
        TODO("STMT_SELECT");
        break;
    case STMT_ITER:
        //display_iter_stmt(s->stmt.comp, indentation + 1);
        TODO("STMT_ITER");
        break;
    case STMT_JMP:{
        indent(indentation + 2);
        printf("%s\n", get_keyword_str(s->stmt.jmp.jmp));
        if(s->stmt.jmp.jmp_expr >= 0) display_expression(s->stmt.jmp.jmp_expr, indentation + 2);
    }
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
    da_append(parser.symbols, symbol, Symbol);
    return out;
}

int find_symbol(const Str name, int required){
    if(parser.symbols.data) for(
        const Symbol* x = (const Symbol*) ((uintptr_t) parser.symbols.data + parser.symbols.size - sizeof(Symbol));
        (uintptr_t) x >= (uintptr_t) parser.symbols.data;
        x-=1){
        if(compare_str(x->name, name)){
            return (int) (((uintptr_t) (x) - (uintptr_t) (parser.symbols.data)) / sizeof(Symbol));
        }
    }
    if(required){
        report_error("could not find symbol '%.*s'", name.size, name.cstr);
    }
    return -1;
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

const char* get_keyword_str(int keyword){
    switch(keyword){
    case KEYW_NONE:     return "KEYW_NONE";
    case KEYW_CONST:    return "KEYW_CONST";
    case KEYW_STATIC:   return "KEYW_STATIC";
    case KEYW_INLINE:   return "KEYW_INLINE";
    case KEYW_EXTERN:   return "KEYW_EXTERN";
    case KEYW_VOID:     return "KEYW_VOID";
    case KEYW_CHAR:     return "KEYW_CHAR";
    case KEYW_INT:      return "KEYW_INT";
    case KEYW_FLOAT:    return "KEYW_FLOAT";
    case KEYW_DOUBLE:   return "KEYW_DOUBLE";
    case KEYW_RETURN:   return "KEYW_RETURN";
    case KEYW_GOTO:     return "KEYW_GOTO";
    case KEYW_CONTINUE: return "KEYW_CONTINUE";
    case KEYW_BREAK:    return "KEYW_BREAK";
    case KEYW_IF:       return "KEYW_IF";
    case KEYW_ELSE:     return "KEYW_ELSE";
    case KEYW_SWITCH:   return "KEYW_SWITCH";
    case KEYW_FOR:      return "KEYW_FOR";
    case KEYW_WHILE:    return "KEYW_WHILE";
    case KEYW_DO:       return "KEYW_DO";
    default:
        report_internal_error("keyword %i not implemented", keyword);
        return NULL;
    }
}

static inline int decl_func(Str name, Variable ret, int argc, Variable* argv){
    ret.flags = SYMFLAG_PLACEHOLDER;
    return declare_symbol(
        (Symbol){
            .name = name,
            ._type = TYPE_FUNC,
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

    for(Token token = next_token(tokenizer); token.type != TKNTYPE_NONE; token = next_token(tokenizer)){

        const int keyword = get_keyword(token);
        if(keyword != KEYW_NONE){
            const int _type = keyword_type(keyword);
            if(_type != TYPE_ERROR){
                const Token name = expect(TKNTYPE_RAW);
                if(peek(0).type == '('){ // parse function arguments
                    skip(1);
                    if(peek(0).type != ')'){
                        printf("in %s:%i:%i:\n", tokenizer.src_file_name, tokenizer.line, tokenizer.column);
                        TODO(function argument parsing);
                    }
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
                        push_decl_stmt(f);
                        da_element(parser.symbols, f, Symbol)->symbol.func.body = parse_compound_statement();
                    }
                    else{
                        expect(';');
                    }
                }
                else{
                    const int var = declare_symbol(make_basic_var(name.value.str, _type));
                    push_decl_stmt(var);
                    if(peek(0).type == '='){
                        const int lval = push_primary_expr(name);
                        skip(1);
                        const int rval = parse_rside_expression();
                        push_expr_stmt(push_binary_expr(lval, '=', rval));
                    }
                    else{
                        expect(';');
                    }
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