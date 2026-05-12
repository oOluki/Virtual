#ifndef BUILDER_C
#define BUILDER_C

#include "builder.h"
#include "../src/parse_instRW.h"

Builder builder;

size_t get_type_size(int _type){
    switch (_type)
    {
    case TYPE_VOID:
    case TYPE_ERROR:
        report_internal_error("TYPE_VOID and TYPE_ERROR are invalid here%c", ' ');
        return 0;
    case TYPE_CHAR:     return sizeof(char);
    //case TYPE_UCHAR:    return sizeof(unsigned char);
    case TYPE_SHORT:    return sizeof(short);
    case TYPE_INT:      return sizeof(int);
    case TYPE_LONG:     return sizeof(long);
    //case TYPE_UINT:     return sizeof(unsigned int);
    case TYPE_FLOAT:    return sizeof(float);
    case TYPE_DOUBLE:   return sizeof(double);
    case TYPE_PTR:      return sizeof(void*);
    // standard varable size 8
    case TYPE_VAR:      return 8;
    case TYPE_FUNC:     return sizeof(int(*)());
    default:
        TODOF("type %i not implemented", _type);
        return 0;
    }
}

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

int print_iia(FILE* f, const IIArg iia){
    switch (iia.type)
    {
    case IIATYPE_NONE:
        return 0;
    case IIATYPE_ULIT:
        fprintf(f, "%u", iia.value.u);
        return 0;
    case IIATYPE_ILIT:
        fprintf(f, "%i", iia.value.i);
        return 0;
    case IIATYPE_FLIT:
        fprintf(f, "%f", iia.value.f);
        return 0;
    case IIATYPE_ZULIT:
        fprintf(f, "%zu", iia.value.zu);
        return 0;
    case IIATYPE_SLIT:
    case IIATYPE_RAW:
        fprintf(f, "%.*s", iia.value.str.size, iia.value.str.cstr);
        return 0;
    case IIATYPE_SYM:{
        const Symbol* const s = da_element(parser.symbols, iia.value.zu, const Symbol);
        fprintf(f, "%.*s", s->name.size, s->name.cstr);
    }
        return 0;
    case IIATYPE_TUPLE:
        fprintf(f, "(%u, %u)", iia.value.tuple.x, iia.value.tuple.y);
        return 0;
    case IIATYPE_III:
        return print_iii(f, iia.value.iii);
    case IIATYPE_ERROR:
    default:
        report_internal_error("iia %i not implemented", iia.type);
        return -1;
    }
}


int print_iii(FILE* f, const III iii){
    fprintf(f, "%s(", get_iiiop_str(iii.iiiop));
    int i = 0;
    for(; i < ARLEN(iii.arg) - 1 &&  iii.arg[i + 1] >= 0; i+=1){
        print_iia(f, *(const IIArg*) da_element(builder.iias, iii.arg[i], IIArg));
        fprintf(f, ", ");
    }
    if(iii.arg[i] >= 0){
        print_iia(f, *(const IIArg*) da_element(builder.iias, iii.arg[i], IIArg));
    }
    fputc(')', f);
    return 0;
}

static inline int push_iia(const IIArg iia){
    const int out = da_len(builder.iias, IIArg);
    da_append(builder.iias, iia, IIArg);
    return out;
}

static inline int push_iii(int op, int arg1, int arg2, int arg3){

    const int iiaindex = da_len(builder.iias, IIArg);

    const IIArg iia = (IIArg){
        .type = IIATYPE_III,
        .value.iii = (III){
            .iiiop  = op,
            .arg[0] = arg1,
            .arg[1] = arg2,
            .arg[2] = arg3
        }
    };

    da_append(builder.iias, iia, IIArg);

    const int iiindex = da_len(builder.iii_index, int);

    da_append(builder.iii_index, iiaindex, int);

    return iiindex;
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

// \returns the index of the iiarg allocated
static int interpret_expression(int expr){
    const Expr* const e = get_expr(expr);
    switch (e->kind)
    {
    case EXPRTYPE_PRIMARY:
        return push_iia((IIArg){.type = e->expr.primary.kind, .value.tknv = e->expr.primary.value});
    case EXPRTYPE_POSTFIX:
        TODO(EXPRTYPE_POSTFIX);
        break;
    case EXPRTYPE_UNARY:
        TODO(EXPRTYPE_UNARY);
        break;
    case EXPRTYPE_BINARY:{
        const int iiindex = interpret_binary_expr(e->expr.bin.left, e->expr.bin.op, e->expr.bin.right);
        int iia; da_pop(&builder.iii_index, iia, const int);
        return iia;
    }
    case EXPRTYPE_CONDITIONAL:
        TODO(EXPRTYPE_CONDITIONAL);
        break;
    
    default:
        report_internal_error("expression %i not implemented", e->kind);
        break;
    }
    return 0;
}

int build_intermediate_interpretation(){

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
        case STMT_EXPR:{
            const int iiaindex = interpret_expression(stmt->stmt.expr);
            const IIArg iia = *da_element(builder.iias, iiaindex, const IIArg);
            if(iia.type == IIATYPE_III){
                push_iii(iia.value.iii.iiiop, iia.value.iii.arg[0], iia.value.iii.arg[1], iia.value.iii.arg[2]);
            }
            else {
                push_iii(IIIOP_NOP, iiaindex, -1, -1);
            }
        }
            break;
        case STMT_COMP:
            push_iii(IIIOP_OPENSCOPE,
                push_iia(
                    (IIArg){
                        .type = IIATYPE_TUPLE,
                        .value.tuple = {stmt->stmt.comp.start, stmt->stmt.comp.end}
                    }
                ),
                -1, -1
            );
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



// ==========================================[BUILD ASSEMBLY] =========================================== //

static int move(int dest_var, int src);

// \returns the register with the result, or 0 if no result was returned by iii, or -1 on failure
static int build_assemble_iii(const III iii);

// returns the register id where this was stored
static int construct_bignum(uint64_t n);

// \param set_as_used sets the taken register as being used
// \returns index of some unused register, or -1 if all are used
static inline int get_unused_register(int set_as_used){
    for(int i = 1; i < REGISTER_SPACE_SIZE / sizeof(Register); i+=1){
        if(0 == (builder.register_use_status & (1 << i))){
            builder.register_use_status |= (1 << i) * (!!set_as_used);
            return i * sizeof(Register);
        }
    }
    return -1;
}

// sets register reg status to being used or available, use 1 for used and 0 for available
// if reg < 0, then all registers status will be set
// \returns reg
static inline int set_register_use_status(int reg, int _1_for_used_and_0_for_available){
    if(reg < 0)
        builder.register_use_status = _1_for_used_and_0_for_available? UINT64_MAX : 0;
    else
        builder.register_use_status |= (1 << (reg / sizeof(Register))) * (!!_1_for_used_and_0_for_available);
    return reg;
}

// \returns a type refering to what type should be used for the instruction based on the arguments types
static inline int get_instruction_type_by_type_precedence(int ltype, int rtype){
    return (ltype > rtype)? ltype : rtype;
}


typedef struct _push_assembly_inst_arg_t {

    int op;
    int arg1;
    int arg2;
    int arg3;
    int op_profile_either_is_register;

} _push_assembly_inst_arg_t;


static void _push_assembly_inst(const _push_assembly_inst_arg_t _inst){
    if(_inst.op > INT16_MAX)
        report_internal_error("inst.op == %i, can't have inst.op > INT16_MAX", _inst.op);

    if(_inst.op >= INST_TOTAL_COUNT || _inst.op == INST_ERROR)
        report_internal_error("inst.op %i is not a valid instruction op code", _inst.op);

    const OpProfile inst_profile = get_inst_profile(_inst.op);
    Inst inst = _inst.op;
    switch (inst_profile)
    {
    case OP_PROFILE_NONE:
        break;
    case OP_PROFILE_R:
        if(_inst.arg1 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg1);
        inst |= (_inst.arg1 & 0xff) << 8;
        break;
    case OP_PROFILE_E:
        if(_inst.op_profile_either_is_register){
            if(_inst.arg1 > 0xff)
                report_internal_error("register argument overflow %i", _inst.arg1);
            inst |= (_inst.arg1 & 0xff) << 8;
            inst |= HINT_REG << 31;
            break;
        }
        inst |= HINT_LIT << 31;
    case OP_PROFILE_L:
        if(_inst.arg1 > 0xffff)
            report_internal_error("literal argument overflow %i", _inst.arg1);
        inst |= (_inst.arg1 & 0xffff) << 8;
        break;
    case OP_PROFILE_RR:
        if(_inst.arg1 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg1);
        if(_inst.arg2 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg2);
        inst |= ((_inst.arg1 & 0xff) << 8) | ((_inst.arg2 & 0xff) << 16);
        break;
    case OP_PROFILE_RL:
        if(_inst.arg1 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg1);
        if(_inst.arg1 > 0xffff)
            report_internal_error("literal argument overflow %i", _inst.arg1);
        inst |= ((_inst.arg1 & 0xff) << 8) | ((_inst.arg2 & 0xffff) << 16);
        break;
    case OP_PROFILE_RRR:
        if(_inst.arg1 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg1);
        if(_inst.arg2 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg2);
        if(_inst.arg3 > 0xff)
            report_internal_error("register argument overflow %i", _inst.arg3);
        inst |= ((_inst.arg1 & 0xff) << 8) | ((_inst.arg2 & 0xff) << 16) | ((_inst.arg3 & 0xff) << 24);
        break;

    
    default:
        TODOF("inst profile %i not implemented", inst_profile);
        break;
    }

    da_append(builder.assembly, inst, Inst);
}

#define push_assembly_inst(...) _push_assembly_inst((_push_assembly_inst_arg_t){__VA_ARGS__})


// puts value in IIArg _iia to register reg
// \param reg if -1 put it in some unused register, the register state won't be altered
// \returns reg
static int put_in_reg(int reg, int _iia){
    const IIArg* const iia = da_element(builder.iias, _iia, const IIArg);

    if(reg < 0)
        reg = get_unused_register(1);

    switch (iia->type)
    {
    case IIATYPE_NONE:
        push_assembly_inst(INST_MOV, reg, R0);
        break;
    case IIATYPE_ULIT:
    case IIATYPE_ILIT:
    case IIATYPE_FLIT:
    case IIATYPE_ZULIT:
        if(iia->value.u64 <= UINT16_MAX){
            push_assembly_inst(INST_MOVV, reg, (uint16_t) iia->value.u64);
            break;
        }
        push_assembly_inst(INST_MOV, reg, construct_bignum(iia->value.u64));
        break;
    case IIATYPE_SLIT:
        TODO(IIATYPE_SLIT);
        break;
    case IIATYPE_RAW:
        ((IIArg*) iia)->value.i = find_symbol(iia->value.str, 1);
    case IIATYPE_SYM:{
        const Symbol* const sym = da_element(parser.symbols, iia->value.i, const Symbol);
        if(sym->_type != TYPE_VAR)
            report_internal_error("expected type %s, got %s instead", get_type_str(TYPE_VAR), get_type_str(sym->_type));
        if(sym->symbol.var.array_len > 0)
            TODO(sym->symbol.var.array_len > 0);
        
        const size_t backwards_offset = builder.stack_size - sym->symbol.var.mempos;
        if(backwards_offset <= UINT16_MAX){
            push_assembly_inst(INST_STACK_GET, reg, (uint16_t) backwards_offset);
            break;
        }
        const int mempos_reg = construct_bignum(sym->symbol.var.mempos);
        push_assembly_inst(INST_GSP, reg, 0, mempos_reg);
        push_assembly_inst(INST_READ, reg, reg);
    }
        break;
    case IIATYPE_TUPLE:
        TODO(IIATYPE_TUPLE);
        break;
    case IIATYPE_III:
        push_assembly_inst(INST_MOV, reg, build_assemble_iii(iia->value.iii));
        break;
    
    case IIATYPE_ERROR:
        report_internal_error("got IIATYPE_ERROR(%i) somehow", iia->type);
        break;
    default:
        TODOF("iia type %i not implemented", iia->type);
        break;
    }

    set_register_use_status(reg, 0);
    return reg;
}

// returns the register id where this was stored
static int construct_bignum(uint64_t n){
    const int base_reg = get_unused_register(0);
    push_assembly_inst(INST_MOV, base_reg, 0);
    for (int reg = base_reg; n; reg += 2)
    {
        push_assembly_inst(INST_MOVV16, reg, n & UINT16_MAX);
        n >>= 16;
    }
    
    return base_reg;
}

static int move(int dest_var, int src){

    const IIArg* const iia = da_element(builder.iias, dest_var, const IIArg);

    if(iia->type != IIATYPE_RAW)
        report_internal_error("to move value to iiarg, expected type IIATYPE_RAW, got %i instead", iia->type);

    const int symid = find_symbol(iia->value.str, 1);

    const Symbol* const sym = da_element(parser.symbols, symid, Symbol);

    if(sym->_type != TYPE_VAR)
        report_internal_error("to move value expected %s, got %s instead", get_type_str(TYPE_VAR), get_type_str(sym->_type));
    
    const int reg = get_unused_register(1);

    const size_t backwards_offset = builder.stack_size - sym->symbol.var.mempos;
    if(backwards_offset <= UINT16_MAX){
        push_assembly_inst(INST_STACK_PUT, put_in_reg(reg, src), (uint16_t) backwards_offset);
        set_register_use_status(reg, 0);
        return reg;
    }
    const int src_reg = put_in_reg(-1, src);
    push_assembly_inst(INST_GSP, reg, 0, sym->symbol.var.mempos);
    push_assembly_inst(INST_WRITE, reg, src_reg, 0);

    set_register_use_status(reg, 0);
    return src_reg;
}

static int push_bin_inst(int op, int left, int right){
    const int rout = get_unused_register(1);
    const int lreg = put_in_reg(-1, left);
    if(left == right){
        set_register_use_status(rout, 0);
        set_register_use_status(lreg, 0);
        push_assembly_inst(op, rout, lreg, lreg);
        return rout;
    }
    const int rreg = put_in_reg(-1, right);
    set_register_use_status(rout, 0);
    set_register_use_status(lreg, 0);
    push_assembly_inst(op, rout, lreg, rreg);
    return rout;
}


static int build_assemble_iii(const III iii){
    switch (iii.iiiop)
    {
    case IIIOP_NOP:
        push_assembly_inst(INST_NOP);
        return 0;
    case IIIOP_VARDECL:{
        const IIArg iia = *da_element(builder.iias, iii.arg[0], const IIArg);
        Symbol* const sym = da_element(parser.symbols, iia.value.u, Symbol);
        if(sym->_type != TYPE_VAR){
            report_internal_error(
                "while declaring variable, expected sym->_type to be %s, got %s instead",
                get_type_str(TYPE_VAR), get_type_str(sym->_type)
            );
        }
        sym->symbol.var.mempos = builder.stack_size;
        const size_t var_size = get_var_size(sym->symbol.var);
        if(var_size <= UINT16_MAX){
            push_assembly_inst(INST_INC, RSP, (int) var_size);
        }
        else{
            const int reg = construct_bignum(var_size);
            push_assembly_inst(INST_ADD, RSP, RSP, reg);
        }
        builder.stack_size += var_size;
    }
        return 0;
    case IIIOP_FUNCDECL:{
        const IIArg iia = *da_element(builder.iias, iii.arg[0], const IIArg);
        Symbol* const sym = da_element(parser.symbols, iia.value.u, Symbol);
        if(sym->_type != TYPE_FUNC){
            report_internal_error(
                "while declaring function, expected sym->_type to be %s, got %s instead",
                get_type_str(TYPE_FUNC), get_type_str(sym->_type)
            );
        }
        sym->symbol.func.body = da_len(builder.assembly, uint32_t);
    }
        return 0;
    case IIIOP_OPENSCOPE:
    case IIIOP_CLOSESCOPE:{
        const IIArg* const iia = da_element(builder.iias, iii.arg[0], const IIArg);
        builder.scope_begin = iia->value.tuple.x;
        builder.scope_end   = iia->value.tuple.y;
        if(iii.iiiop == IIIOP_OPENSCOPE)
            da_append(builder.stack_bases, builder.stack_size, size_t);
        else{
            const size_t old_stack_size = builder.stack_size;
            da_pop(&builder.stack_bases, builder.stack_size, size_t);
            if(old_stack_size < builder.stack_size){
                report_error("old stack size %zu is smaller then after closing scope %zu\n",
                    old_stack_size, builder.stack_size);
                return -1;
            }
            const size_t sub = old_stack_size - builder.stack_size;
            if(sub == 0)
                return 0;
            if(sub <= UINT16_MAX){
                push_assembly_inst(INST_DEC, RSP, (int) sub);
            }
            else{
                const int reg = construct_bignum(sub);
                push_assembly_inst(INST_SUB, RSP, RSP, reg);
            }
        }
    }
        return 0;
    case IIIOP_MOV:
        return move(iii.arg[0], iii.arg[1]);
    // case IIIOP_READ:
    // case IIIOP_WRITE:
    // case IIIOP_NOT:
    // case IIIOP_NEG:
    // case IIIOP_AND:
    // case IIIOP_LAND:
    // case IIIOP_NAND:
    // case IIIOP_OR:
    // case IIIOP_LOR:
    // case IIIOP_XOR:
    // case IIIOP_LSHIFT:
    // case IIIOP_RSHIFT:
    // case IIIOP_NEGEQ:
    // case IIIOP_ANDEQ:
    // case IIIOP_LANDEQ:
    // case IIIOP_OREQ:
    // case IIIOP_LOREQ:
    // case IIIOP_XOREQ:
    // case IIIOP_LSEQ:
    // case IIIOP_RSEQ:
    // case IIIOP_JMP:
    // case IIIOP_JMPF:
    // case IIIOP_JMPFN:
    // case IIIOP_CALL:
    case IIIOP_ADD:
        return push_bin_inst(INST_ADD, iii.arg[0], iii.arg[1]);
    case IIIOP_SUB:
        return push_bin_inst(INST_SUB, iii.arg[0], iii.arg[1]);
    case IIIOP_MUL:
        return push_bin_inst(INST_MUL, iii.arg[0], iii.arg[1]);
    case IIIOP_DIV:
        return push_bin_inst(INST_DIVU, iii.arg[0], iii.arg[1]);
    // case IIIOP_ADDEQ:
    // case IIIOP_SUBEQ:
    // case IIIOP_MULEQ:
    // case IIIOP_DIVEQ:
    // case IIIOP_INC:
    // case IIIOP_DEC:
    // case IIIOP_ABS:
    // case IIIOP_NEQ:
    // case IIIOP_EQ:
    // case IIIOP_BIG:
    // case IIIOP_SML:
    // case IIIOP_CASTI:
    // case IIIOP_CASTF:
    // case IIIOP_CASTU:
    // case IIIOP_SYS:
    case IIIOP_RET:
        put_in_reg(RX, iii.arg[0]);
        set_register_use_status(RX, 1);
        {   // closing scope...
            const size_t old_stack_size = builder.stack_size;
            da_pop(&builder.stack_bases, builder.stack_size, size_t);
            if(old_stack_size < builder.stack_size){
                report_error("old stack size %zu is smaller then after closing scope %zu\n",
                    old_stack_size, builder.stack_size);
                return -1;
            }
            const size_t sub = old_stack_size - builder.stack_size;
            if(sub > 0 && sub <= UINT16_MAX){
                push_assembly_inst(INST_DEC, RSP, (int) sub);
            }
            else if(sub > 0){
                const int reg = construct_bignum(sub);
                push_assembly_inst(INST_SUB, RSP, RSP, reg);
            }
        }
        set_register_use_status(-1, 0);
        push_assembly_inst(INST_RET);
        return RX;
    case IIIOP_EXIT:
        put_in_reg(RA, iii.arg[0]);
        push_assembly_inst(INST_HALT, RA);
        return RA;

    case IIIOP_ERROR:
        report_internal_error("got %s somehow", "IIIOP_EXIT");
        return -1;
    default:
        TODOF("iiiop %i not implemented", iii.iiiop);
        return -1;
    }
    return 0;
}

int build_assembly(){

    const int mainid = find_symbol((Str){.cstr = "main", .size = sizeof("main") - sizeof(char)}, 0);
    if(mainid < 0){
        fprintf(stderr, "[ERROR] missing entry point 'main'\n");
        return 1;
    }
    const Symbol* const main = da_element(parser.symbols, mainid, const Symbol);

    if(main->_type != TYPE_FUNC){
        fprintf(
            stderr, "[ERROR] entry point 'main' should have type %s, got %s instead\n",
            get_type_str(TYPE_FUNC), get_type_str(main->_type)
        );
        return 1;
    }
    
    size_t entry_point = main->symbol.func.body;

    size_t i = 0;

    for(; i < da_len(builder.iii_index, int) && i < entry_point; i+=1){
        const int iiindex = *da_element(builder.iii_index, i, const int);
        const IIArg* const iia = da_element(builder.iias, iiindex, const IIArg);
        if(iia->type != IIATYPE_III){
            report_internal_error("iii of invalid type %i", iia->type);
        }
        
        const III iii = iia->value.iii;

        build_assemble_iii(iii);

    }

    if(i == entry_point){
        entry_point = da_len(builder.assembly, Inst);
    }

    for(; i < da_len(builder.iii_index, int); i+=1){
        const int iiindex = *da_element(builder.iii_index, i, const int);
        const IIArg* const iia = da_element(builder.iias, iiindex, const IIArg);
        if(iia->type != IIATYPE_III){
            report_internal_error("iii of invalid type %i", iia->type);
        }
        
        const III iii = iia->value.iii;

        build_assemble_iii(iii);

    }

    const size_t actual_entry_point = da_len(builder.assembly, Inst);


    const int reg = construct_bignum(entry_point);

    push_assembly_inst(INST_SUB, reg, reg, RIP);

    push_assembly_inst(INST_DEC, reg, 2);

    push_assembly_inst(INST_CALL, reg, .op_profile_either_is_register=1);

    push_assembly_inst(INST_HALT, RX, .op_profile_either_is_register=1);

    da_append(builder.assembly, actual_entry_point, uint64_t);

    set_register_use_status(-1, 0);

    return 0;
}


#endif // =====================  END OF FILE BUILDER_C ===========================