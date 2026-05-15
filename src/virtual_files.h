#ifndef VIRTUAL_FILES_H
#define VIRTUAL_FILES_H

#include "core.h"

#define VIRTUAL_FILE_MAGIC_NUMBER "VF:"

#define VIRTUAL_FILE_INTERNAL_FLAG_IS_LITTLE_ENDIAN 1

#define VIRTUAL_FILE_PROGRAM_FIELD_NAME "program"
#define VIRTUAL_FILE_LABELS_FIELD_NAME "labels"
#define VIRTUAL_FILE_STATIC_FIELD_NAME "static"

enum VirtualFileTypes{
    VIRTUAL_FILE_TYPE_UNKNOWN = 0,
    VIRTUAL_FILE_TYPE_EXE,

    // for counting purposes
    VIRTUAL_FILE_TYPE_COUNT
};

typedef struct VirtualFile
{
    int         validated;
    const char* name;
    uint8_t     file_flags;
    // for now this is just padding to garantee that if the file is read into an 8 byte aligned buffer
    // the beginning of the file's data will also be aligned to 8 bytes
    uint8_t     xflag;
    uint16_t    vfile_type;
    uint64_t    field_count;
    uint64_t    file_data_size;
    uint64_t*   fields;
    void*       data;
} VirtualFile;


int query_field(const char* field, const char** field_array);

// \param field_base_ptr the base pointer from which to take the fields positions from,
// pass NULL if fields are to be interpreted as RAW pointers instead of relative offsets
VirtualFile create_virtual_file(
    const char* name,
    uint8_t  file_flags,
    uint16_t vfile_type,
    uint64_t data_size,
    uint64_t field_count,
    const void* fields_base_ptr,
    const uint64_t* fields
);

int vfopen(VirtualFile* vfile, const char* path, const char** required_fields, const char** optional_fields);

// \param name the fields name (up to 8 characters including null termination), if the name is already in data pass NULL
// \param data_size the data size in bytes
int add_virtual_file_field(VirtualFile* vfile, const char* name, uint64_t data_size, const void* data);

void* get_virtual_file_field(const VirtualFile vfile, const char* field);

void vfclose(VirtualFile vfile);


Inst* get_program_from_vfield(const void* _field, uint64_t* _program_size, uint64_t* entry_point);

int vfsave(const VirtualFile vfile, const char* path);


#endif // =====================  END OF FILE VIRTUAL_FILES_H ===========================