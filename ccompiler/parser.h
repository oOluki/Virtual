#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"


enum Keywords{
    KEYW_NONE = 0,

    KEYW_CONST,
    KEYW_STATIC,
    KEYW_INLINE,
    KEYW_EXTERN,
    KEYW_VOID,
    KEYW_CHAR,
    KEYW_INT,
    KEYW_FLOAT,
    KEYW_DOUBLE,
    
    KEYW_RETURN,
    KEYW_GOTO,
    KEYW_CONTINUE,
    KEYW_BREAK,

    KEYW_IF,
    KEYW_ELSE,
    KEYW_SWITCH,

    KEYW_FOR,
    KEYW_WHILE,
    KEYW_DO,
};

enum VariableType{
    VARTYPE_NONE = 0,
    VARTYPE_BASIC,
    VARTYPE_ARR,

};

enum SymFlag{
    SYMFLAG_NONE        = 0,
    SYMFLAG_CONST       = 1 << 0,
    SYMFLAG_STATIC      = 1 << 1,
    SYMFLAG_EXTERN      = 1 << 2,
    SYMFLAG_UNSIGNED    = 1 << 3,
    SYMFLAG_INLINE      = 1 << 4,
    
    SYMFLAG_PLACEHOLDER = 1 << 7
};

enum ExprType{
    EXPRTYPE_NONE = 0,
    EXPRTYPE_PRIMARY,
    EXPRTYPE_POSTFIX,
    EXPRTYPE_UNARY,
    EXPRTYPE_BINARY,
    EXPRTYPE_CONDITIONAL,
};

enum StatementType {
    STMT_NONE = 0,
    STMT_DECL,
    STMT_EXPR,
    STMT_COMP,
    STMT_SELECT,
    STMT_ITER,
    STMT_JMP
};

enum IntermediateInterpretationInstructionOPCode {
    IIIOP_NONE,

    IIIOP_VARDECL,

    IIIOP_RET,

    IIIOP_EXIT,
};

enum What {

    PREPROCESS_INCLUDE,
    PREPROCESS_DEF,
    PREPROCESS_IF,

    FUNC_DECLARATION,
    FUNC_DEFINITION,

    VAR_DECLARATION,    
};

enum IntermediateInterpretationArgumentType{
    IIATYPE_NONE = 0,
    IIATYPE_ULIT,
    IIATYPE_ILIT,
    IIATYPE_FLIT,
    IIATYPE_VAR,

    IIATYPE_ERROR

};


typedef struct IntermediateInterpretationArgument
{
    int   type;
    TokenValue value;
} IIArg;

typedef struct IntermediateInterpretationInstruction
{
    int     iiiop;
    IIArg   arg[3];

} III;

typedef union VariableValue
{
    char            c;
    unsigned char   uc;
    int             i;
    unsigned int    u;
    float           f;
    double          lf;
    void*           p;
} VarVal;


typedef struct BasicVariable
{
    int _type;
    VarVal val;
} Bvar;


typedef struct ArrayVariable
{
    int base_type;
    int pointer_counter;
    VarVal* value;
    unsigned int array_len;
} ArrVar;


typedef struct Variable
{
    int _type;
    int flags;
    union
    {
        Bvar    basic;
        ArrVar  arr;
    } var;
    int pointer_counter;
    int array_len;
} Variable;

typedef struct Function
{
    Variable    ret;
    int         argc;
    Variable*   args;
    int         body;
} Function;


typedef struct Symbol
{
    Str name;
    int _type;
    union SymbolData
    {
        Function func;
        Variable var;
    } symbol;
    
} Symbol;


typedef struct PrimaryExpression{
    int kind;
    TokenValue value;
} PriExpr;

typedef struct PostFixExpression{
    int left;
    int middle;
    int right;
} PostExpr;

typedef struct UnaryExpression{
    int what;
    int expr;
} UnaryExpr;

typedef struct BinaryExpression{
    int op;
    int left;
    int right;
} BinExpr;

typedef struct ConditionalExpression{
    int cond;
    int if_;
    int else_;
} CondExpr;

typedef struct Expression
{
    int kind;
    union
    {
        PriExpr     primary;
        PostExpr    postfix;
        UnaryExpr   unary;
        BinExpr     bin;
        CondExpr    cond;
    } expr;
    
} Expr;

typedef struct SelectStatement
{
    int if_expr;
    int if_stmt;
    int else_stmt;    
} SelStmt;

typedef struct IterStatement
{
    int cond;
    int stmt;    
} IterStmt;

typedef struct JumpStatement
{
    int jmp;
    int jmp_expr;  
} JmpStmt;

typedef struct Statement
{
    int kind;
    union
    {
        struct{
            int start;
            int end;
        }           comp;
        int         decl;
        BinExpr     expr;
        SelStmt     sel;
        IterStmt    iter;
        JmpStmt     jmp;
    } stmt;
    
} Statement;



typedef struct Parser
{
    DyArr       symbols;
    DyArr       expressions;
    DyArr       statements;
    DyArr       instructions;
} Parser;

int display_tree(int start, int end);

#define make_basic_var(NAME, TYPE) ((Symbol){.name = NAME, ._type = TYPE_VAR, .symbol.var = (Variable){._type = VARTYPE_BASIC, .var.basic._type = (TYPE)}})

int declare_symbol(const Symbol symbol);

int find_symbol(const Str name, int required);

// print Intermediate Interpretation Argument
int print_iia(IIArg iia);

// print Intermediate Interpretation Instruction
int print_iii(III iii);


int get_keyword(Token token);

int keyword_type(int keyword);

const char* get_keyword_str(int keyword);

// expressions

int push_primary_expr(const Token token);
int push_postfix_expr(int left, int postop, int right);
int push_unary_expr(int what, int expr);
int push_binary_expr(int left, int binop, int right);
int push_cond_expr(int cond_expr, int if_expr, int else_expr);

int parse_lside_expression(const Token token);

int parse_rside_expression();

Expr* get_expr(int expr);

// statements

int push_decl_stmt(int symbol);
int push_expr_stmt(int lexpr, int middle_op, int rexpr);
int push_comp_stmt(int start, int end);
int push_select_stmt(int if_expr, int if_stmt, int else_stmt);
int push_iter_stmt(int cond, int iter_stmt);
int push_jump_stmt(int jmp_keyword, int jmp_expr);

int parse_decl_statement(int first_identifier);
int parse_expression_statement(const Token first);
int parse_compound_statement();
int parse_selection_statement(int if_switch);
int parse_iteration_statement(int for_while_do);
int parse_jump_statement(int jmp_keyword);

Statement* get_stmt(int stmt);

// ...

int parse_file(const char* file);


extern Parser parser;

#endif // =====================  END OF FILE PARSER_H ===========================