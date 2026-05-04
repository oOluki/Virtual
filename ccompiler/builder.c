#ifndef BUILDER_C
#define BUILDER_C

#include "builder.h"


Builder builder;

const char* get_iiiop_str(int iiiop){
    switch (iiiop)
    {
    case IIIOP_NOP:         return "NOP";
    case IIIOP_VARDECL:     return "VARDECL";
    case IIIOP_FUNCDECL:    return "FUNCDECL";
    case IIIOP_OPENSCOPE:   return "OPENSCOPE";
    case IIIOP_CLOSESCOPE:  return "CLOSESCOPE";
    case IIIOP_MOV:         return "MOV";
    case IIIOP_READ:        return "READ";
    case IIIOP_WRITE:       return "WRITE";
    case IIIOP_NOT:         return "NOT";
    case IIIOP_NEG:         return "NEG";
    case IIIOP_AND:         return "AND";
    case IIIOP_LAND:        return "LAND";
    case IIIOP_NAND:        return "NAND";
    case IIIOP_OR:          return "OR";
    case IIIOP_LOR:         return "LOR";
    case IIIOP_XOR:         return "XOR";
    case IIIOP_LSHIFT:      return "LSHIFT";
    case IIIOP_RSHIFT:      return "RSHIFT";
    case IIIOP_NEGEQ:       return "IIIOP_NEGEQ";
    case IIIOP_ANDEQ:       return "IIIOP_ANDEQ";
    case IIIOP_LANDEQ:      return "IIIOP_LANDEQ";
    case IIIOP_OREQ:        return "IIIOP_OREQ";
    case IIIOP_LOREQ:       return "IIIOP_LOREQ";
    case IIIOP_XOREQ:       return "IIIOP_XOREQ";
    case IIIOP_LSEQ:        return "IIIOP_LSEQ";
    case IIIOP_RSEQ:        return "IIIOP_RSEQ";
    case IIIOP_JMP:         return "JMP";
    case IIIOP_JMPF:        return "JMPF";
    case IIIOP_JMPFN:       return "JMPFN";
    case IIIOP_CALL:        return "CALL";
    case IIIOP_ADD:         return "ADD";
    case IIIOP_SUB:         return "SUB";
    case IIIOP_MUL:         return "MUL";
    case IIIOP_DIV:         return "DIV";
    case IIIOP_ADDEQ:       return "ADDEQ";
    case IIIOP_SUBEQ:       return "SUBEQ";
    case IIIOP_MULEQ:       return "MULEQ";
    case IIIOP_DIVEQ:       return "DIVEQ";
    case IIIOP_INC:         return "INC";
    case IIIOP_DEC:         return "DEC";
    case IIIOP_ABS:         return "ABS";
    case IIIOP_NEQ:         return "NEQ";
    case IIIOP_EQ:          return "EQ";
    case IIIOP_BIG:         return "BIG";
    case IIIOP_SML:         return "SML";
    case IIIOP_CASTI:       return "CASTIU";
    case IIIOP_CASTF:       return "CASTIF";
    case IIIOP_CASTU:       return "CASTUI";
    case IIIOP_SYS:         return "SYS";
    case IIIOP_RET:         return "RET";
    case IIIOP_EXIT:        return "EXIT";   
    case IIIOP_ERROR:       return "ERROR";

    default:
        report_internal_error("iiiop %i not implemented", iiiop);
        return NULL;
    }
}

int get_iiiop_from_str(const Str str){

    if(comp_str_cstr(str, "NOP"))        return IIIOP_NOP;
    if(comp_str_cstr(str, "VARDECL"))    return IIIOP_VARDECL;
    if(comp_str_cstr(str, "FUNCDECL"))   return IIIOP_FUNCDECL;
    if(comp_str_cstr(str, "MOV"))        return IIIOP_MOV;
    if(comp_str_cstr(str, "READ"))       return IIIOP_READ;
    if(comp_str_cstr(str, "WRITE"))      return IIIOP_WRITE;
    if(comp_str_cstr(str, "NOT"))        return IIIOP_NOT;
    if(comp_str_cstr(str, "NEG"))        return IIIOP_NEG;
    if(comp_str_cstr(str, "AND"))        return IIIOP_AND;
    if(comp_str_cstr(str, "LAND"))       return IIIOP_LAND;
    if(comp_str_cstr(str, "NAND"))       return IIIOP_NAND;
    if(comp_str_cstr(str, "OR"))         return IIIOP_OR;
    if(comp_str_cstr(str, "LOR"))        return IIIOP_LOR;
    if(comp_str_cstr(str, "XOR"))        return IIIOP_XOR;
    if(comp_str_cstr(str, "LSHIFT"))     return IIIOP_LSHIFT;
    if(comp_str_cstr(str, "RSHIFT"))     return IIIOP_RSHIFT;
    if(comp_str_cstr(str, "NEGEQ"))      return IIIOP_NEGEQ;
    if(comp_str_cstr(str, "ANDEQ"))      return IIIOP_ANDEQ;
    if(comp_str_cstr(str, "LANDEQ"))     return IIIOP_LANDEQ;
    if(comp_str_cstr(str, "OREQ"))       return IIIOP_OREQ;
    if(comp_str_cstr(str, "LOREQ"))      return IIIOP_LOREQ;
    if(comp_str_cstr(str, "XOREQ"))      return IIIOP_XOREQ;
    if(comp_str_cstr(str, "LSEQ"))       return IIIOP_LSEQ;
    if(comp_str_cstr(str, "RSEQ"))       return IIIOP_RSEQ;
    if(comp_str_cstr(str, "JMP"))        return IIIOP_JMP;
    if(comp_str_cstr(str, "JMPF"))       return IIIOP_JMPF;
    if(comp_str_cstr(str, "JMPFN"))      return IIIOP_JMPFN;
    if(comp_str_cstr(str, "CALL"))       return IIIOP_CALL;
    if(comp_str_cstr(str, "ADD"))        return IIIOP_ADD;
    if(comp_str_cstr(str, "SUB"))        return IIIOP_SUB;
    if(comp_str_cstr(str, "MUL"))        return IIIOP_MUL;
    if(comp_str_cstr(str, "DIV"))        return IIIOP_DIV;
    if(comp_str_cstr(str, "ADDEQ"))      return IIIOP_ADDEQ;
    if(comp_str_cstr(str, "SUBEQ"))      return IIIOP_SUBEQ;
    if(comp_str_cstr(str, "MULEQ"))      return IIIOP_MULEQ;
    if(comp_str_cstr(str, "DIVEQ"))      return IIIOP_DIVEQ;
    if(comp_str_cstr(str, "INC"))        return IIIOP_INC;
    if(comp_str_cstr(str, "DEC"))        return IIIOP_DEC;
    if(comp_str_cstr(str, "ABS"))        return IIIOP_ABS;
    if(comp_str_cstr(str, "NEQ"))        return IIIOP_NEQ;
    if(comp_str_cstr(str, "EQ"))         return IIIOP_EQ;
    if(comp_str_cstr(str, "BIG"))        return IIIOP_BIG;
    if(comp_str_cstr(str, "SML"))        return IIIOP_SML;
    if(comp_str_cstr(str, "CASTIU"))     return IIIOP_CASTI;
    if(comp_str_cstr(str, "CASTIF"))     return IIIOP_CASTF;
    if(comp_str_cstr(str, "CASTUI"))     return IIIOP_CASTU;
    if(comp_str_cstr(str, "SYS"))        return IIIOP_SYS;
    if(comp_str_cstr(str, "RET"))        return IIIOP_RET;
    if(comp_str_cstr(str, "EXIT"))       return IIIOP_EXIT;

    return IIIOP_ERROR;
}

// print Intermediate Interpretation Argument
int print_iia(const IIArg iia){
    switch (iia.type)
    {
    case IIATYPE_NONE:
        return 0;
    case IIATYPE_ULIT:
        return printf("%u", iia.value.u);
    case IIATYPE_ILIT:
        return printf("%i", iia.value.i);
    case IIATYPE_FLIT:
        return printf("%f", iia.value.f);
    case IIATYPE_ZULIT:
        return printf("%zu", iia.value.zu);
    case IIATYPE_SLIT:
    case IIATYPE_RAW:
        return printf("%.*s", iia.value.str.size, iia.value.str.cstr);
    case IIATYPE_SYM:{
        const Symbol* const s = da_element(parser.symbols, iia.value.zu, const Symbol);
        return printf("%.*s", s->name.size, s->name.cstr);
    }
    case IIATYPE_III:
        return print_iii(*(III*) iia.value.p);
    case IIATYPE_ERROR:
    default:
        report_internal_error("iia %i not implemented", iia.type);
        return 1;
    }
}


// print Intermediate Interpretation Instruction
int print_iii(const III iii){
    printf("%s(", get_iiiop_str(iii.iiiop));
    int i = 0;
    for(; i < ARLEN(iii.arg) - 1 &&  iii.arg[i + 1] >= 0; i+=1){
        print_iia(*(const IIArg*) da_element(builder.iias, iii.arg[i], IIArg));
        printf(", ");
    }
    if(iii.arg[i] >= 0){
        print_iia(*(const IIArg*) da_element(builder.iias, iii.arg[i], IIArg));
    }
    printf(")");
}

static inline int push_iia(const IIArg iia){
    const int out = da_len(builder.iias, IIArg);
    da_append(builder.iias, iia, IIArg);
    return out;
}

static inline int push_iii(int op, int arg1, int arg2, int arg3){

    const III iii = (III){
        .iiiop  = op,
        .arg[0] = arg1,
        .arg[1] = arg2,
        .arg[2] = arg3
    };
    const int index = builder.iiis.size / sizeof(III);
    da_append(builder.iiis, iii, III);

    return index;
}

static int interpret_expression(int expr);

static int interpret_postfix_expr(int left, int postop, int right){
    TODO(interpret_postfix_expr);
    return 0;
}
static int interpret_unary_expr(int what, int expr){
    TODO(interpret_unary_expr);
    return 0;
}
static inline int interpret_binary_expr(int left, int binop, int right){
    return push_iii(binop, interpret_expression(left), interpret_expression(right), -1);
}
static int interpret_cond_expr(int cond_expr, int if_expr, int else_expr){
    TODO(interpret_cond_expr);
    return 0;
}
static int interpret_expression(int expr){
    const Expr* const e = get_expr(expr);
    switch (e->kind)
    {
    case EXPRTYPE_PRIMARY:
        return push_iia((IIArg){.type = e->expr.primary.kind, .value = e->expr.primary.value});
    case EXPRTYPE_POSTFIX:
        TODO(EXPRTYPE_POSTFIX);
        break;
    case EXPRTYPE_UNARY:
        TODO(EXPRTYPE_UNARY);
        break;
    case EXPRTYPE_BINARY:
        TODO(EXPRTYPE_BINARY);
        break;
    case EXPRTYPE_CONDITIONAL:
        TODO(EXPRTYPE_CONDITIONAL);
        break;
    
    default:
        report_internal_error("expression %i not implemented", e->kind);
        break;
    }
    return 0;
}

int interpret(){

    for(size_t i = 0; i < parser.statements.size / sizeof(Statement); i+=1){
        const Statement* const stmt = da_element(parser.statements, i, const Statement);
        switch (stmt->kind)
        {
        case STMT_NONE:
            break;
        case STMT_DECL:{
            const Symbol* const sym = da_element(parser.symbols, stmt->stmt.decl, const Symbol);
            push_iii(
                (sym->_type == TYPE_VAR)? IIIOP_VARDECL : IIIOP_FUNCDECL,
                push_iia((IIArg){.type = IIATYPE_SYM, .value.u = stmt->stmt.decl}),
                -1, -1
            );
        }
            break;
        case STMT_EXPR:
            interpret_binary_expr(stmt->stmt.expr.left, stmt->stmt.expr.op, stmt->stmt.expr.right);
            break;
        case STMT_COMP:
            push_iii(IIIOP_OPENSCOPE, push_iia((IIArg){.type = IIATYPE_ZULIT, .value.zu = builder.scope}), -1, -1);
            break;
        case STMT_SELECT:
            TODO(STMT_SELECT);
            break;
        case STMT_ITER:
            TODO(STMT_ITER);
            break;
        case STMT_JMP:{
            switch (stmt->stmt.jmp.jmp)
            {
            case KEYW_RETURN:
                push_iii(IIIOP_RET, interpret_expression(stmt->stmt.jmp.jmp_expr), -1, -1);
                break;
            case KEYW_GOTO:
                push_iii(IIIOP_JMP, push_iia((IIArg){.type = IIATYPE_SYM, .value.zu = stmt->stmt.jmp.jmp_expr}), -1, -1);
                break;
            case KEYW_CONTINUE:
                TODO(CONTINUE);
                break;
            case KEYW_BREAK:
                TODO(BREAK);
                break;
            
            default:
                report_internal_error("keyword %i not implemented\n", stmt->stmt.jmp.jmp);
                break;
            }
        }
            break;
            
        default:
            report_internal_error("statement kind %i not implemented", stmt->kind);
            return 1;
        }
    }

    return 0;
}



#endif // =====================  END OF FILE BUILDER_C ===========================