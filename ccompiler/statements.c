#ifndef STATEMENTS_C
#define STATEMENTS_C

#include "parser.h"

static inline int push_stmt(const Statement stmt){
    const int out = parser.statements.size / sizeof(Statement);
    da_append(parser.statements, stmt, Statement);
    return out;
}

int push_decl_stmt(int symbol){
    return push_stmt((Statement){.kind = STMT_DECL, .stmt.decl = symbol});
}

int push_expr_stmt(int expr){
    return push_stmt(
        (Statement){.kind = STMT_EXPR, .stmt.expr = expr}
    );
}
int push_comp_stmt(int start, int end){
    return push_stmt((Statement){.kind = STMT_COMP, .stmt.comp.start = start, .stmt.comp.end = end});
}
int push_select_stmt(int if_expr, int if_stmt, int else_stmt){
    return push_stmt(
        (Statement){
            .kind = STMT_SELECT,
            .stmt.sel.if_expr = if_expr, .stmt.sel.if_stmt = if_stmt, .stmt.sel.else_stmt = else_stmt
        }
    );
}
int push_iter_stmt(int cond, int iter_stmt){
    return push_stmt((Statement){.kind = STMT_ITER, .stmt.iter.cond = cond, .stmt.iter.stmt = iter_stmt});
}
int push_jump_stmt(int jmp_keyword, int jmp_expr){
    return push_stmt((Statement){.kind = STMT_JMP, .stmt.jmp.jmp = jmp_keyword, .stmt.jmp.jmp_expr = jmp_expr});
}

static int parse_vardecl_statement(int first_identifier){
    const Token name = expect(TKNTYPE_RAW);
    const int var = declare_symbol(make_basic_var(name.value.str, first_identifier));
    return push_decl_stmt(var);
}

int parse_expression_statement(const Token first){
    const int lval = parse_lside_expression(first);
    const Token t = next_token();
    if(t.type == ';'){
        return push_expr_stmt(lval);
    }
    else if(is_token_assign(t.type)){
        const int rval = parse_rside_expression();
        return push_expr_stmt(push_binary_expr(lval, t.type, rval));
    }
    else
        report_error("invalid expression%c", ' ');
    return -1;
}

int parse_compound_statement(){

    const int start = parser.statements.size / sizeof(Statement);

    const int out = push_comp_stmt(start, 0);

    Token token = next_token();

    for(; token.type != TKNTYPE_NONE && token.type != '}'; token = next_token()){

        if(token.type == ';') continue;

        const int keyword = get_keyword(token);
        if(keyword != KEYW_NONE){
            const int _type = keyword_type(keyword);
            if(_type != TYPE_ERROR){
                const Token name = expect(TKNTYPE_RAW);
                const int var = declare_symbol(make_basic_var(name.value.str, _type));
                push_decl_stmt(var);
                if(peek(0).type == '='){
                    skip(1);
                    push_expr_stmt(push_binary_expr(push_primary_expr(name), '=', parse_rside_expression()));
                }
                else{
                    expect(';');
                }
            }
            else if(keyword == KEYW_RETURN){
                parse_jump_statement(keyword);
            }
        }
        else if(token.type == TKNTYPE_RAW){
            parse_expression_statement(token);
        }
        else{
            report_error("invalid syntax%c", ' ');
        }

    }

    if(token.type != '}'){
        report_error("missing closing %c", '}');
    }

    get_stmt(out)->stmt.comp.end = parser.statements.size / sizeof(Statement);

    return out;
}




int parse_jump_statement(int jmp_keyword){

    switch (jmp_keyword)
    {
    case KEYW_GOTO:{
        const int label = find_symbol(expect(TKNTYPE_RAW).value.str, 1);
        return push_jump_stmt(jmp_keyword, label);
    }
    case KEYW_RETURN:{
        const int expr = parse_rside_expression();
        return push_jump_stmt(jmp_keyword, expr);
    }
    case KEYW_CONTINUE:
    case KEYW_BREAK:
        return push_jump_stmt(jmp_keyword, -1);
    
    default:
        report_internal_error("%i jmp_keyword not implemented", jmp_keyword);
        break;
    }

    return 0;
}

Statement* get_stmt(int stmt){
    if(stmt < 0 || stmt >= parser.statements.size / sizeof(Statement))
        report_internal_error(
            "attempted to get non existent statement(%i), there were %i statements",
            stmt, (int) (parser.statements.size / sizeof(Statement))
        );
    return da_element(parser.statements, stmt, Statement);
}


#endif // =====================  END OF FILE STATEMENTS_C ===========================