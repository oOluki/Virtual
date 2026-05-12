#ifndef EXPRESSION_C
#define EXPRESSION_C

#include "parser.h"

static inline int push_expr(const Expr expr){
    const int out = parser.expressions.size / sizeof(Expr);
    da_append(parser.expressions, expr, Expr);
    return out;
}

int push_primary_expr(const Token token){
    if(token.type != TKNTYPE_RAW && token.type != TKNTYPE_STR && !is_token_literal(token.type)){
        report_error("expected primary expression, got %s instead", get_tkntype_str(token.type));
    }
    return push_expr((Expr){.kind = EXPRTYPE_PRIMARY, .expr.primary.kind = token.type, .expr.primary.value = token.value});
}

int push_postfix_expr(int left, int postop, int right){
    return push_expr((Expr){.kind = EXPRTYPE_POSTFIX, .expr.postfix.left = left, .expr.postfix.middle = postop, .expr.postfix.right = right});
}
int push_unary_expr(int what, int expr){
    return push_expr((Expr){.kind = EXPRTYPE_UNARY, .expr.unary.what = what, .expr.unary.expr = expr});
}
int push_binary_expr(int left, int binop, int right){
    return push_expr((Expr){.kind = EXPRTYPE_BINARY, .expr.bin.left = left, .expr.bin.op = binop, .expr.bin.right = right});
}
int push_cond_expr(int cond_expr, int if_expr, int else_expr){
    return push_expr((Expr){.kind = EXPRTYPE_CONDITIONAL, .expr.cond.cond = cond_expr, .expr.cond.if_ = if_expr, .expr.cond.else_ = else_expr});
}


static int validade_lvalue(int lvalue){
    return lvalue >= 0;
}

int parse_lside_expression(const Token token){

    if(token.type != TKNTYPE_RAW){
        report_error("expected variable name, got %s instead", get_tkntype_str(token.type));
    }

    const int sym = find_symbol(token.value.str, 1);

    return push_primary_expr(token);
}

int parse_rside_expression(){

    Token token = next_token();

    int expr = 0;

    if(!is_token_literal(token.type) && token.type != TKNTYPE_RAW){
        if(token.type == '(')
            TODO(token.type == '(');
        report_error(
            "in %s:%i:%i: expected literal or variable, got %s instead\n",
            tokenizer.src_file_name, tokenizer.line, tokenizer.column, get_tkntype_str(token.type)
        );
    }

    if(is_token_operand(peek(0).type)){
        const int binop = next_token().type;
        expr = push_binary_expr(push_primary_expr(token), binop, parse_rside_expression());
    }
    else expr = push_primary_expr(token);

    return expr;
}


Expr* get_expr(int expr){
    if(expr < 0 || expr >= parser.expressions.size / sizeof(Expr))
        report_internal_error(
            "attempted to get non existent expression(%i), there were %i expressions",
            expr, (int) (parser.expressions.size / sizeof(Expr))
        );
    return da_element(parser.expressions, expr, Expr);
}



#endif // =====================  END OF FILE EXPRESSION_C ===========================