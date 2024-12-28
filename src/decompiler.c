#include "core.h"
#include "lexer.h"
#include <stdio.h>
#include <inttypes.h>

static char buff1[10];
static char buff2[10];
static char buff3[10];

typedef struct Exe
{
    Mc_stream_t data;
    uint64_t    entry_point;
    uint64_t    meta_data_size;
    void*       meta_data;
    uint32_t    flags;
} Exe;


char get_digit_char(int i){
    switch (i)
    {
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';

    default: return '\0';
    }
}

char* get_reg_str(int reg, char* output){



    output[3] = '\0';
    switch (reg)
    {
    case RA1:
    case RA2:
    case RA3:
    case RA4:
    case RA:
        output[0] = 'R';
        output[1] = 'A';
        output[2] = get_digit_char(reg - RA);
        return output;
    case RB1:
    case RB2:
    case RB3:
    case RB4:
    case RB:
        output[0] = 'R';
        output[1] = 'B';
        output[2] = get_digit_char(reg - RB);
        return output;
    case RC1:
    case RC2:
    case RC3:
    case RC4:
    case RC:
        output[0] = 'R';
        output[1] = 'C';
        output[2] = get_digit_char(reg - RC);
        return output;
    case RD1:
    case RD2:
    case RD3:
    case RD4:
    case RD:
        output[0] = 'R';
        output[1] = 'D';
        output[2] = get_digit_char(reg - RD);
        return output;
    case RE1:
    case RE2:
    case RE3:
    case RE4:
    case RE:
        output[0] = 'R';
        output[1] = 'E';
        output[2] = get_digit_char(reg - RE);
        return output;
    case RF:
        output[0] = 'R';
        output[1] = 'F';
        output[2] = '\0';
        return output;
    case RSP:
        output[0] = 'R';
        output[1] = 'S';
        output[2] = 'P';
        return output;
    case RIP:
        output[0] = 'R';
        output[1] = 'I';
        output[2] = 'P';
        return output;
    
    default: return NULL;
    }
}

#define READ_AS(TYPE, PDATA) *(TYPE*)(PDATA)

int print_inst(uint8_t inst, const char* data, char* static_memory){
    switch (inst)
    {
    case INST_NOP:
        printf("NOP\n");
        return 1;
    case INST_HALT:
        printf("HALT\n");
        return 1;
    case INST_MOV8:
        printf("MOV8 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MOV16:
        printf("MOV16 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MOV32:
        printf("MOV32 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MOV:
        printf("MOV %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MOVV:{
        Register op = {.as_uint64 = *(uint64_t*)(data + 1)};
        printf("MOVV %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(data[0], buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 10;
    case INST_PUSH:
        printf("PUSH %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_PUSHV:{
        Register op = {.as_uint64 = *(uint64_t*)(data + 1)};
        printf("PUSHV (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 9;
    case INST_POP:
        printf("POP %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_GET:
        printf("READ %s\n", get_reg_str(data[0], buff1));
        return 10;
    case INST_WRITE:
        printf("WRITE %s\n", get_reg_str(data[0], buff1));
        return 10;
    case INST_GSP:
        printf("GSP %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_STATIC:
        printf("STATIC %"PRIx64" \"%*s\"...\n", *(uint64_t*)(data), 10, static_memory + *(uint64_t*)(data));
        return 9;
    case INST_READ8:
        printf("READ8 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[0], buff2));
        return 3;
    case INST_READ16:
        printf("READ16 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[0], buff2));
        return 3;
    case INST_READ32:
        printf("READ32 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[0], buff2));
        return 3;
    case INST_READ:
        printf("READ %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[0], buff2));
        return 3;
    case INST_SET8:
        printf("SET8 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SET16:
        printf("SET16 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SET32:
        printf("SET32 %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SET:
        printf("SET %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_TEST:
        printf("TEST %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_NOT:
        printf("NOT %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_NEG:
        printf("NEG %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_AND:
        printf("AND %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 2;
    case INST_NAND:
        printf("NAND %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 2;
    case INST_OR:
        printf("OR %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 2;
    case INST_XOR:
        printf("XOR %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 2;
    case INST_BSHIFT:
        printf("BSHIFT %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 2;
    case INST_JMP:
        printf("JMP %s\n", get_reg_str(data[0], buff1));
        return 9;
    case INST_JMPIF:
        printf("JMPF %s %"PRIu64"\n", get_reg_str(data[0], buff1), *(uint64_t*)(data + 1));
        return 10;
    case INST_JMPIFN:
        printf("JMPFN %s %"PRIu64"\n", get_reg_str(data[0], buff1), *(uint64_t*)(data + 1));
        return 10;
    case INST_CALL:
        printf("CALL %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_RET:
        printf("RET\n");
        return 1;
    case INST_ADD:
        printf("ADD %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SUB:
        printf("SUB %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MUL:
        printf("MUL %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_DIVI:
        printf("DIVI %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_DIVU:
        printf("DIVU %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_ADDF:
        printf("ADDF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SUBF:
        printf("SUBF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_MULF:
        printf("MULF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_DIVF:
        printf("DIVF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_EQI:
        printf("EQI %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_EQU:
        printf("EQU %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_EQF:
        printf("EQF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_BIGI:
        printf("BIGI %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_BIGU:
        printf("BIGU %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_BIGF:
        printf("BIGF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SMLI:
        printf("SMLI %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SMLU:
        printf("SMLU %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_SMLF:
        printf("SMLF %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_CASTIU:
        printf("CASTIU %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CASTIF:
        printf("CASTIF %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CASTUI:
        printf("CASTUI %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CASTUF:
        printf("CASTUF %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CASTFI:
        printf("CASTFI %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CASTFU:
        printf("CASTFU %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CF3264:
        printf("CF3264 %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_CF6432:
        printf("CF6432 %s\n", get_reg_str(data[0], buff1));
        return 2;

    case INST_MEMSET:
        printf("MEMSET %s %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2), get_reg_str(data[2], buff3));
        return 4;
    case INST_MEMCPY:
        printf("MEMCPY %s %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2), get_reg_str(data[2], buff3));
        return 4;
    case INST_MEMMOV:
        printf("MEMMOV %s %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2), get_reg_str(data[2], buff3));
        return 4;
    case INST_MEMCMP:
        printf("MEMCMP %s %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2), get_reg_str(data[2], buff3));
        return 4;
    case INST_MALLOC:
        printf("MALLOC %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_FREE:
        printf("FREE %s\n", get_reg_str(data[0], buff1));
        return 2;
    
    case INST_FOPEN:
        printf("FOPEN %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_FCLOSE:
        printf("FCLOSE %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_PUTC:
        printf("PUTC %s %s\n", get_reg_str(data[0], buff1), get_reg_str(data[1], buff2));
        return 3;
    case INST_GETC:
        printf("GETC %s\n", get_reg_str(data[0], buff1));
        return 2;
    

    case INST_DISREG:
        printf("DISREG %s\n", get_reg_str(data[0], buff1));
        return 2;
    case INST_SYS:
        printf("SYS %02"PRIx64"\n", *(uint64_t*)data);
        return 9;
    
    
    default:
        fprintf(stderr, "[ERROR] Unkonwn Instruction '%u'\n", inst);
        exit(3);
        return 0;
    }
}


int main(int argc, char** argv){

    if(argc != 2){
        fprintf(stderr, "[ERROR] Expected 1 Argument, Got %i Instead\n", argc - 1);
        return 1;
    }

    Mc_stream_t stream = mc_create_stream(1000);

    if(!read_file(&stream, argv[1], "rb")){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", argv[1]);
        mc_destroy_stream(stream);
        return 2;
    }

    uint32_t flags;
    uint64_t meta_data_size;
    uint64_t entry_point;

    if(get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags)) return 1;

    char* static_memory = NULL;

    for(size_t i = 16; i + 8 < 16 + meta_data_size; ){
        const uint64_t size = *(uint64_t*)(stream.data + i);
        const uint64_t id   = *(uint64_t*)(stream.data + i + sizeof(uint64_t));
        if(id == is_little_endian()? mc_swap64(0x5354415449433a) : 0x5354415449433a){
            static_memory = stream.data + i;
            break;
        }
        i+=size;
    }
    const size_t static_memory_size = (size_t)(*(uint64_t*)(static_memory));

    const size_t program_size = stream.size - 24 - meta_data_size;

    printf(
        "\nX====X (SPECIFICATIONS) X====X\n"
        "\tname = %s\n"
        "\ttotal size     = %zu\n"
        "\tprogram size   = %zu\n"
        "\tflags          = %02x\n"
        "\tmeta_data_size = %zu\n"
        "\tstatic_memory = { position = %zu, pointer = %p, size = %zu }\n"
        "\tentry point    = %zu\n"
        "X====X (SPECIFICATIONS) X====X\n\n\n",
        argv[1],
        (size_t)stream.size,
        program_size,
        flags,
        (size_t)(meta_data_size),
        (size_t)(static_memory - (char*)stream.data), static_memory, static_memory_size,
        (size_t)(entry_point)
    );

    const size_t start = 24 + meta_data_size + entry_point;

    for(
        size_t i = 0;
        i < program_size;
        i += print_inst(
            ((unsigned char*)stream.data)[i + start],
            stream.data + i + start + 1, static_memory
        )
    );


    return 0;
}




