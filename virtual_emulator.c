/*

MIT License

Copyright (c) 2024 oOluki

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



*/




#include <stdlib.h>
#include <stdio.h>
#include "core.h"

#define STACK_CAP 1024

enum States{
    STATE_NONE = 0,

    STATE_ACTIVE
};


struct VirtualMachine{

unsigned char* internal_memory;
size_t internal_memory_size;

Var* stack;
size_t stack_size;

size_t ip;
} vm;


void load_program(const char* path, Vm_flags flags){
    FILE* file = fopen(path, "rb");

    if(!file){
		printf("[ERROR] Invalid File Path '%s'\n", path);
		exit(ERROR_INVALID_FILE_PATH);
	}

    Stream stream = (Stream){.data = malloc(1024), .size = 0, .capacity = 1024};

    unsigned char meta[32];

	if(fread(meta, 4 * sizeof(char) + 2 * sizeof(uint64_t), 1, file) != 1){
		throw_error(ERROR_INVALID_EXECUTABLE, NULL);
	}

	if((meta[0] != 'v') || (meta[1] != 'f') || (meta[2] != 'e') || (meta[3] != ':')){
		fprintf(stderr, "[ERROR] Incorrect Magic Number\n");
    	throw_error(ERROR_INVALID_EXECUTABLE, NULL);
	}

	const uint64_t program_start_position = *(uint64_t*)(meta + 4 * sizeof(char));
    const uint64_t entry_point = *(uint64_t*)(meta + 4 * sizeof(char) + sizeof(uint64_t));


    for(; !feof(file);){
        stream.size += (SIZEOF_CHUNK) * fread(stream.data + stream.size, SIZEOF_CHUNK, stream.capacity / SIZEOF_CHUNK, file);
        if(stream.size >= stream.capacity){
            resize_stream(&stream, 2 * stream.capacity);
        }
    }

    fclose(file);

    vm.internal_memory = (unsigned char*)malloc(stream.size + STACK_CAP - program_start_position);
    vm.internal_memory_size = stream.size - program_start_position;
    
    vm.stack = (Var*)(vm.internal_memory + stream.size - program_start_position);
    vm.stack_size = 0;

    vm.ip = entry_point;

    for(size_t i = 0; i < vm.internal_memory_size; i += 1){
        vm.internal_memory[i] = stream.data[i + program_start_position];
    }

    free(stream.data);
}

// returns the number of bytes taken by the instruction, so to be convinient to jump to next instruction
size_t eval_inst(size_t inst_address){
    const Inst inst = *(Inst*)(vm.internal_memory + inst_address);
    switch (inst)
    {
    case INSTRUCTION_HALT:
        exit(0);
        return sizeof(Inst);
    case INSTRUCTION_DUMP_STACK:
        printf("stack:\n");
        for(size_t i = 0; i < vm.stack_size; i+=1){
            const Var var = vm.stack[i];
            printf(
                "\t%" PRIu64 "-- i: %" PRId64 ", u: %" PRIu64 ", f: %f, ptr: %p\n",
                (uint64_t)i, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
            );
        }
        return sizeof(Inst);
    case INSTRUCTION_GSP:
        vm.stack[vm.stack_size].as_uint64 = vm.stack_size - 1;
        vm.stack_size += 1;
        return sizeof(Inst);
    case INSTRUCTION_GSO:
        vm.stack[vm.stack_size].as_uint64 = vm.stack_size - 1 - (*(Var*)(vm.internal_memory + inst_address + sizeof(Inst))).as_uint64;
        vm.stack_size += 1;
        return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_IP:
        vm.stack[vm.stack_size++].as_uint64 = inst_address + sizeof(Inst);
        return sizeof(Inst);

    case INSTRUCTION_PUSH:
        vm.stack[vm.stack_size++] =
        *(Var*)(vm.internal_memory + inst_address + sizeof(Inst));
        return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_POP:
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_CLEAN:
        vm.stack_size = 0;
        return sizeof(Inst);
    case INSTRUCTION_DUP:
        vm.stack[vm.stack_size] = vm.stack[vm.stack_size - 1];
        vm.stack_size += 1;
        return sizeof(Inst);
    case INSTRUCTION_READ:{
        const size_t i = vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack[vm.stack_size - 1] = vm.stack[i];
    } return sizeof(Inst);
    case INSTRUCTION_READS:{
        vm.stack[vm.stack_size] =
        vm.stack[vm.stack_size - 1 - (*(Var*)(vm.internal_memory + inst_address + sizeof(Inst))).as_uint64];
        vm.stack_size += 1;
    } return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_SET:
        vm.stack[vm.stack[vm.stack_size - 2].as_uint64] =
        vm.stack[vm.stack[vm.stack_size - 1].as_uint64];
        vm.stack_size -= 2;
        return sizeof(Inst);
    case INSTRUCTION_SWAP:{
        const Var dummy = vm.stack[vm.stack_size - 1];
        vm.stack[vm.stack_size - 1] = vm.stack[vm.stack_size - 2];
        vm.stack[vm.stack_size - 2] = dummy;
    } return sizeof(Inst);
    case INSTRUCTION_BSHIFT:{
        const int64_t shift = vm.stack[vm.stack_size - 1].as_int64;
        if(shift > 0)
            vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 << shift;
        else
            vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 >> (-shift);
        vm.stack_size -= 1;
    } return sizeof(Inst);
    
    case INSTRUCTION_NOT:
        vm.stack[vm.stack_size - 1].as_uint64 = vm.stack[vm.stack_size - 1].as_uint64? 0 : 1;
        return sizeof(Inst);
    case INSTRUCTION_EQUAL:
        vm.stack[vm.stack_size - 2].as_uint64 =
        (vm.stack[vm.stack_size - 2].as_uint64 == vm.stack[vm.stack_size - 1].as_uint64)? 1 : 0;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_SNEQUAL:
        vm.stack[vm.stack_size - 2].as_uint64 =
        (vm.stack[vm.stack_size - 2].as_int64 == vm.stack[vm.stack_size - 1].as_int64)? 1 : 0;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_FEQUAL:
        vm.stack[vm.stack_size - 2].as_uint64 =
        (vm.stack[vm.stack_size - 2].as_float64 == vm.stack[vm.stack_size - 1].as_float64)? 1 : 0;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_JMP:
        vm.ip = vm.stack[vm.stack_size-- - 1].as_uint64;
        return 0;
    case INSTRUCTION_JMP_IF:
        if(vm.stack[vm.stack_size - 2].as_uint64){
            vm.ip = vm.stack[vm.stack_size - 1].as_uint64;
            vm.stack_size -= 2;
            return 0;
        }
        vm.stack_size -= 2;
        return sizeof(Inst);
    case INSTRUCTION_JMP_IFNOT:
        if(vm.stack[vm.stack_size - 2].as_uint64 == 0){
            vm.ip = vm.stack[vm.stack_size - 1].as_uint64;
            vm.stack_size -= 2;
            return 0;
        }
        vm.stack_size -= 2;
        return sizeof(Inst);
    case INSTRUCTION_CALL:
        vm.stack[vm.stack_size++].as_uint64 = vm.ip + sizeof(Inst) + sizeof(Var);
        vm.ip = (*(Var*)(vm.internal_memory + inst_address + sizeof(Inst))).as_uint64;
        return 0;
    case INSTRUCTION_CALLIF:
        vm.stack[vm.stack_size++].as_uint64 = vm.ip + sizeof(Inst) + sizeof(Var);
        if(vm.stack[vm.stack_size - 1].as_uint64){
            vm.ip = (*(Var*)(vm.internal_memory + inst_address + sizeof(Inst))).as_uint64;
            vm.stack_size -= 1;
            return 0;
        }
        vm.stack_size -= 1;
        return sizeof(Inst) + sizeof(Var);
    case INSTRUCTION_CALLNIF:
        vm.stack[vm.stack_size++].as_uint64 = vm.ip + sizeof(Inst) + sizeof(Var);
        if(vm.stack[vm.stack_size - 1].as_uint64 == 0){
            vm.ip = (*(Var*)(vm.internal_memory + inst_address + sizeof(Inst))).as_uint64;
            vm.stack_size -= 1;
            return 0;
        }
        vm.stack_size -= 1;
        return sizeof(Inst) + sizeof(Var);

    case INSTRUCTION_PLUSI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 1].as_int64 + vm.stack[vm.stack_size - 2].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 - vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 * vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVI:
        vm.stack[vm.stack_size - 2].as_int64 = vm.stack[vm.stack_size - 2].as_int64 / vm.stack[vm.stack_size - 1].as_int64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_SMALLERI:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_int64 < vm.stack[vm.stack_size - 1].as_int64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_BIGGERI:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_int64 > vm.stack[vm.stack_size - 1].as_int64);
        vm.stack_size -= 1;
        return sizeof(Inst);

    case INSTRUCTION_PLUSU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 1].as_uint64 + vm.stack[vm.stack_size - 2].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 - vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 * vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVU:
        vm.stack[vm.stack_size - 2].as_uint64 = vm.stack[vm.stack_size - 2].as_uint64 / vm.stack[vm.stack_size - 1].as_uint64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_SMALLERU:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_uint64 < vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_BIGGERU:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_uint64 > vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);

    case INSTRUCTION_PLUSF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 1].as_float64 + vm.stack[vm.stack_size - 2].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MINUSF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 - vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_MULF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 * vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_DIVF:
        vm.stack[vm.stack_size - 2].as_float64 = vm.stack[vm.stack_size - 2].as_float64 / vm.stack[vm.stack_size - 1].as_float64;
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_SMALLERF:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_float64 < vm.stack[vm.stack_size - 1].as_float64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_BIGGERF:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_float64 > vm.stack[vm.stack_size - 1].as_float64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    
    case INSTRUCTION_CAST_I2U:
        vm.stack[vm.stack_size - 1].as_uint64 = (uint64_t)(vm.stack[vm.stack_size - 1].as_int64);
        return sizeof(Inst);
    case INSTRUCTION_CAST_I2F:
        vm.stack[vm.stack_size - 1].as_float64 = (double)(vm.stack[vm.stack_size - 1].as_int64);
        return sizeof(Inst);
    case INSTRUCTION_CAST_U2I:
        vm.stack[vm.stack_size - 1].as_int64 = (int64_t)(vm.stack[vm.stack_size - 1].as_uint64);
        return sizeof(Inst);
    case INSTRUCTION_CAST_U2F:
        vm.stack[vm.stack_size - 1].as_float64 = (double)(vm.stack[vm.stack_size - 1].as_uint64);
        return sizeof(Inst);
    case INSTRUCTION_CAST_F2I:
        vm.stack[vm.stack_size - 1].as_int64 = (int64_t)(vm.stack[vm.stack_size - 1].as_float64);
        return sizeof(Inst);
    case INSTRUCTION_CAST_F2U:
        vm.stack[vm.stack_size - 1].as_uint64 = (uint64_t)(vm.stack[vm.stack_size - 1].as_float64);
        return sizeof(Inst);
    
    case INSTRUCTION_AND:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_uint64 & vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_OR:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_uint64 | vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_XOR:
        vm.stack[vm.stack_size - 2].as_uint64 = (vm.stack[vm.stack_size - 2].as_uint64 ^ vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_NAND:
        vm.stack[vm.stack_size - 2].as_uint64 = ~(vm.stack[vm.stack_size - 2].as_uint64 & vm.stack[vm.stack_size - 1].as_uint64);
        vm.stack_size -= 1;
        return sizeof(Inst);
    case INSTRUCTION_NEGATE:
        vm.stack[vm.stack_size - 1].as_uint64 = ~(vm.stack[vm.stack_size - 1].as_uint64);
        return sizeof(Inst);

    default:
        printf("[ERROR] At Instruction Address %zu Unkown Instruction %u\n", vm.ip, (unsigned int)inst);
        exit(ERROR_UNKOWN_INSTRUCTION);
        return sizeof(Inst);
    }
}

void print_help_message(){
    printf("[HELP] Usage: ./vimulator <path_to_executable> <optional: flags>\n");
    printf("[HELP] Usage ./vimulator --help to display this help message\n");
    printf(
        "[HELP] flags can be:\n"
        "    '-d': run in degub mode\n"
    );
}

static inline int ipow(int base, int exp){
    int output = 1;
    for(int i = 0; i < exp; i += 1){
        output *= base;
    }
    return output;
}

int get_int_fromstr(char* str){
    int output = 0;
    int str_size = 0;
    for(; str[str_size] && str[str_size] != ' ' && str[str_size] != '\n'; str_size += 1);

    for(int i = 0; i < str_size; i+=1){
        int digit = get_int(str[i]);
        if(digit < 0) return -1;
        output += digit * ipow(10, str_size - i - 1);
    }

    return output;
}

int main(int argc, char** argv){

	if(argc < 2){
		fprintf(stderr, "[ERROR] Expected At Least One Argument, Got NONE Instead\n");
        printf("[HELP] Use ./vimulator --help to display a help message\n");
		return ERROR_INVALID_USAGE;
	}

    Vm_flags flags = VM_FLAG_STD;

    if(argc == 2 && argv[1][0] == '-'){ // checking for the special case ./vimulator --help
        size_t argv_size = 0;
        for(; argv[1][argv_size]; argv_size += 1);
        const String argv_str = (String){.c_str = argv[1], .size = argv_size};
        if(compare_str(argv_str, MKSTR("--help"))){
            print_help_message();
            return 0;
        }
    }

    for(int i = 2; i < argc; i += 1){ // checking for special flags
        size_t argv_size = 0;
        for(; argv[i][argv_size]; argv_size += 1);
        const String argv_str = (String){.c_str = argv[i], .size = argv_size};
        if(compare_str(argv_str, MKSTR("--help"))){
            print_help_message();
        } else if(compare_str(argv_str, MKSTR("-d"))){
            flags |= VM_FLAG_DEBUG;
        } else{
            fprintf(stderr, "[ERROR] Invalid Flag '%s'\n", argv[i]);
            return 1;
        }
    }

	load_program(argv[1], flags);

    if(flags & VM_FLAG_DEBUG){
        printf(
            "[DEBUG] enter:\n"
            "'e' to exit\n"
            "'c' to clear the screen; "
            "'h' to display this message\n"
            "'s' to display stack; "
            "'i' to display instruction; "
            "'n<count>' to evaluate the next <count> instructions. if <count> = -1: evaluate to the end of the program\n"
        );
        do{
            // getting input
            char c = fgetc(stdin);

            if(feof(stdin)) break;

            if(c == '\n') continue;
            size_t stride = 0;
            // evaluating
            switch (c)
            {
            case 'h':
                printf(
                    "[DEBUG] enter:\n"
                    "'e' to exit\n"
                    "'c' to clear the screen; "
                    "'h' to display this message\n"
                    "'s' to display stack; "
                    "'i' to display instruction; "
                    "'n<count>' to evaluate the next <count> instructions. if <count> = -1: evaluate to the end of the program\n"
                );
                break;
            case 'c':

                #ifdef _WIN32
                    system("cls");
                #else 
                    system("clear");
                #endif

                break;
            case 's':
                printf("stack:\n");
                for(size_t i = 0; i < vm.stack_size; i+=1){
                    const Var var = vm.stack[i];
                    printf(
                        "\t%" PRIu64 "-- i: %" PRId64 ", u: %" PRIu64 ", f: %f, ptr: %p\n",
                        (uint64_t)i, var.as_int64, var.as_uint64, var.as_float64, var.as_ptr
                    );
                }
                break;
            case 'i':
                printf("inst %zu:\n\t", vm.ip);
                print_inst(
                    (Program){
                        .data = vm.internal_memory,
                        .size = vm.internal_memory_size,
                        .capacity = vm.internal_memory_size
                    }, vm.ip
                );
                break;
            case 'n':{
                char __strN_str[11];
                __strN_str[10] = '\0';
                fgets(__strN_str, 10, stdin);
                if(__strN_str[0] == '\n'){ // case of a stand alone 'n'
                    stride = 1;
                    break;
                } else if(__strN_str[0] == '-'){
                    stride = (size_t)(-1);
                    break;
                }
                const int N = get_int_fromstr(__strN_str);
                if(N < 0){
                    printf("[DEBUG] ERROR: invalid input '%s'\n", __strN_str);
                    break;
                }
                stride = N;
            } break;
            case 'e':
                return 0;
            
            default:
                printf("[DEBUG] ERROR: invalid input '%c'\n", c);
                break;
            }
            
            for(
                size_t i = 0;
                i++ < stride && vm.ip < vm.internal_memory_size;
                vm.ip += eval_inst(vm.ip)
            );
        } while(vm.ip < vm.internal_memory_size);
    }
    else {
        for(
        ;
        vm.ip < vm.internal_memory_size;
        vm.ip += eval_inst(vm.ip)
        );
    }


    return 0;
}

