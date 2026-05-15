#ifndef _VPU_ASSEMBLER_H
#define _VPU_ASSEMBLER_H

#include "virtual.h"


int write_exe(const Mc_stream_t* program, const char* path, uint64_t entry_point, const Mc_stream_t static_memory,
    const Mc_stream_t labels, uint64_t flags);

#ifdef _WIN32
// assembles program in input_path to output_path
int assemble(char* input_path, char* output_path, int export_all_labels);
#else
// assembles program in input_path to output_path
int assemble(const char* input_path, const char* output_path, int export_all_labels);
#endif


#endif // END OF FILE ============================================