#ifndef VDISASSEMBLER_H
#define VDISASSEMBLER_H

#include "core.h"
#include <stdio.h>


// \param buff should be an array of 3 buffers of size 8 bytes each
// \returns 0 on success or 1 otherwise
int print_inst(FILE* output, Inst inst, char** buff);

// disassembles program in input_path, writing the result to output_path
// \returns 0 on success or error identifier on failure
int disassemble(
    FILE* output,
    const Inst* program, uint64_t inst_count, uint64_t entry_point,
    const void* labels, uint64_t labels_byte_size,
    const void* static_memory, uint64_t static_memory_size
);

int disassemble_file(const char* input, const char* output);

#endif // =====================  END OF FILE VDISASSEMBLER_H ===========================