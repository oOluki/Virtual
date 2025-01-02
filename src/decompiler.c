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
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';

    default: return '\0';
    }
}

char* get_reg_str(int reg, char* output){

    switch (8 * (int)(reg / 8))
    {
    case RA:
        output[0] = 'R';
        output[1] = 'A';
        output[2] = get_digit_char(reg - RA);
        output[3] = '\0';
        return output;
    case RB:
        output[0] = 'R';
        output[1] = 'B';
        output[2] = get_digit_char(reg - RB);
        output[3] = '\0';
        return output;
    case RC:
        output[0] = 'R';
        output[1] = 'C';
        output[2] = get_digit_char(reg - RC);
        output[3] = '\0';
        return output;
    case RD:
        output[0] = 'R';
        output[1] = 'D';
        output[2] = get_digit_char(reg - RD);
        output[3] = '\0';
        return output;
    case RE:
        output[0] = 'R';
        output[1] = 'E';
        output[2] = get_digit_char(reg - RE);
        output[3] = '\0';
        return output;
    case RF:
        output[0] = 'R';
        output[1] = 'F';
        output[2] = get_digit_char(reg - RF);
        output[3] = '\0';
        return output;
    case RSP:
        output[0] = 'R';
        output[1] = 'S';
        output[2] = 'P';
        output[3] = get_digit_char(reg - RSP);
        output[4] = '\0';
        return output;
    case RIP:
        output[0] = 'R';
        output[1] = 'I';
        output[2] = 'P';
        output[2] = get_digit_char(reg - RIP);
        output[4] = '\0';
        return output;
    
    default: return NULL;
    }
}

// \returns 0 on success or 1 otherwise
int print_inst(Inst inst, const uint8_t* static_memory){
    #define R1 (uint8_t) ((inst & 0XFF00)      >> 8)
    #define R2 (uint8_t) ((inst & 0XFF0000)    >> 16)
    #define R3 (uint8_t) ((inst & 0XFF000000)  >> 24)
    #define L2 (uint16_t) ((inst & 0XFFFF0000) >> 16)
    #define L  (uint32_t) ((inst & 0XFFFFFF00) >> 8)
    switch (inst & 0XFF)
    {
    case INST_NOP:
        printf("NOP\n");
        return 0;
    case INST_HALT:
        printf("HALT\n");
        return 0;
    case INST_MOV8:
        printf("MOV8 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV16:
        printf("MOV16 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV32:
        printf("MOV32 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOV:
        printf("MOV %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MOVC:
        printf("MOVC %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R3, buff3));
        return 0;
    case INST_MOVV:{
        Register op = {.as_uint16 = L2};
        printf("MOVV %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVN:{
        Register op = {.as_uint16 = L2};
        printf("MOVN %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_MOVV16:{
        Register op = {.as_uint16 = L2};
        printf("MASK %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_PUSH:
        printf("PUSH %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_PUSHV:{
        Register op = {.as_uint16 = L2};
        printf("PUSHV (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_POP:
        printf("POP %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_GET:{
        Register op = {.as_uint64 = L2};
        printf("GET %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_WRITE:{
        Register op = {.as_uint64 = L2};
        printf("WRITE %s (%02"PRIx64"; u: %"PRIu64"; i: %"PRIi64"; f: %f)\n", get_reg_str(R1, buff1), op.as_uint64, op.as_uint64, op.as_int64, op.as_float64);
    }   return 0;
    case INST_GSP:
        printf("GSP %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_STATIC:{
        const char* string = (char*)(static_memory + L);
        const uint64_t max_size = *(uint64_t*)(static_memory) - L;
        printf("STATIC %"PRIu32" \"%.*s\"...\n", L, (15 < max_size)? (int)15 : (int)max_size, string);
    }   return 0;
    case INST_READ8:
        printf("READ8 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_READ16:
        printf("READ16 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_READ32:
        printf("READ32 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_READ:
        printf("READ %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SET8:
        printf("SET8 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SET16:
        printf("SET16 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SET32:
        printf("SET32 %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SET:
        printf("SET %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_TEST:
        printf("TEST %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_NOT:
        printf("NOT %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_NEG:
        printf("NEG %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_AND:

        printf("AND %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_NAND:
        printf("NAND %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_OR:
        printf("OR %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_XOR:
        printf("XOR %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_BSHIFT:
        printf("BSHIFT %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_JMP:
        printf("JMP %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_JMPIF:
        printf("JMPF %s %"PRIu64"\n", get_reg_str(R1, buff1), L2);
        return 0;
    case INST_JMPIFN:
        printf("JMPFN %s %"PRIu64"\n", get_reg_str(R1, buff1), L2);
        return 0;
    case INST_CALL:
        printf("CALL %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_RET:
        printf("RET\n");
        return 0;
    case INST_ADD:
        printf("ADD %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SUB:
        printf("SUB %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MUL:
        printf("MUL %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_DIVI:
        printf("DIVI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_DIVU:
        printf("DIVU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_ADDF:
        printf("ADDF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SUBF:
        printf("SUBF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_MULF:
        printf("MULF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_DIVF:
        printf("DIVF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_EQI:
        printf("EQI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_EQU:
        printf("EQU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_EQF:
        printf("EQF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_BIGI:
        printf("BIGI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_BIGU:
        printf("BIGU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_BIGF:
        printf("BIGF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SMLI:
        printf("SMLI %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SMLU:
        printf("SMLU %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_SMLF:
        printf("SMLF %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_CASTIU:
        printf("CASTIU %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CASTIF:
        printf("CASTIF %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CASTUI:
        printf("CASTUI %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CASTUF:
        printf("CASTUF %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CASTFI:
        printf("CASTFI %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CASTFU:
        printf("CASTFU %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CF3264:
        printf("CF3264 %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_CF6432:
        printf("CF6432 %s\n", get_reg_str(R1, buff1));
        return 0;

    case INST_MEMSET:
        printf("MEMSET %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R2, buff3));
        return 0;
    case INST_MEMCPY:
        printf("MEMCPY %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R2, buff3));
        return 0;
    case INST_MEMMOV:
        printf("MEMMOV %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R2, buff3));
        return 0;
    case INST_MEMCMP:
        printf("MEMCMP %s %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2), get_reg_str(R2, buff3));
        return 0;
    case INST_MALLOC:
        printf("MALLOC %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_FREE:
        printf("FREE %s\n", get_reg_str(R1, buff1));
        return 0;
    
    case INST_FOPEN:
        printf("FOPEN %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_FCLOSE:
        printf("FCLOSE %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_PUTC:
        printf("PUTC %s %s\n", get_reg_str(R1, buff1), get_reg_str(R2, buff2));
        return 0;
    case INST_GETC:
        printf("GETC %s\n", get_reg_str(R1, buff1));
        return 0;
    

    case INST_DISREG:
        printf("DISREG %s\n", get_reg_str(R1, buff1));
        return 0;
    case INST_SYS:
        printf("SYS %02"PRIx32"\n", L);
        return 0;
    
    
    default:
        fprintf(stderr, "[ERROR] Unkonwn Instruction OpCode %u\n", inst & 0xFF);
        return 1;
    }
}


int main(int argc, char** argv){

    #if 0 // X==X (DEBUG) X==X
    argc = 2;
    argv = malloc(argc * sizeof(char*));
    argv[1] = "../output.bin";
    #endif

    if(argc != 2){
        fprintf(stderr, "[ERROR] Expected 1 Argument, Got %i Instead\n", argc - 1);
        return 1;
    }

    Mc_stream_t stream = mc_create_stream(1024);

    if(!read_file(&stream, argv[1], 1, 0)){
        fprintf(stderr, "[ERROR] Could Not Open/Read '%s'\n", argv[1]);
        mc_destroy_stream(stream);
        return 2;
    }

    uint32_t padding;
    uint64_t flags;
    uint64_t entry_point;
    uint64_t meta_data_size;

    const uint64_t skip = sizeof(uint32_t) + sizeof(padding) + sizeof(flags) + sizeof(entry_point) + sizeof(meta_data_size);

    uint8_t* meta_data = get_exe_specifications(stream.data, &meta_data_size, &entry_point, &flags, &padding);

    if(meta_data == NULL) return 1;

    uint8_t* static_memory = NULL;

    for(size_t i = 0; i + 8 < meta_data_size; ){
        const uint64_t size = *(uint64_t*)(meta_data + i);
        const uint64_t id   = *(uint64_t*)(meta_data + i + sizeof(uint64_t));
        if(id == is_little_endian()? mc_swap64(0x5354415449433a) : 0x5354415449433a){
            static_memory = meta_data + i;
            break;
        }
        i+=size;
    }
    const uint64_t static_memory_size = *(uint64_t*)(static_memory);

    const uint64_t inst_count = (stream.size - skip - meta_data_size - padding) / sizeof(Inst);

    printf(
        "\nX====X (SPECIFICATIONS) X====X\n"
        "\tname = %s\n"
        "\texecutable size = %"PRIu64"\n"
        "\tinst count      = %"PRIu64"\n"
        "\tinst size       = %"PRIu32"\n"
        "\tflags           = %02"PRIx64"\n"
        "\tmeta_data_size  = %"PRIu64"\n"
        "\tstatic_memory   = { position = %"PRIu64", pointer = %p, size = %"PRIu64" }\n"
        "\tentry point     = %"PRIu64"\n"
        "X====X (SPECIFICATIONS) X====X\n\n\n",
        argv[1],
        stream.size,
        inst_count,
        (uint32_t) sizeof(Inst),
        flags,
        meta_data_size,
        (uint64_t)(size_t)(static_memory - meta_data), static_memory, static_memory_size,
        entry_point
    );

    const Inst* program = (Inst*)((uint8_t*)(stream.data) + skip + meta_data_size + padding);

    int status = 0;
    for(size_t i = 0; (i < inst_count) && !status; i += 1)
        status = print_inst(program[i], static_memory);


    return status;
}




