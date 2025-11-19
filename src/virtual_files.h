#ifndef VIRTUAL_FILES_H
#define VIRTUAL_FILES_H

#include "core.h"
#include "virtual.h"

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


int query_field(const char* field, const char** field_array){

    if(field_array == NULL) return -1;

    for(int i = 0; field_array[i]; i+=1){
        if(mc_compare_str(field, field_array[i], 0)){
            int j = i;
            for(; field_array[j + 1]; j+=1){
                field_array[j] = field_array[j + 1];
            }
            field_array[j] = NULL;
            return i;
        }
    }
    return -1;
}

// \param field_base_ptr the base pointer from which to take the fields positions from,
// pass NULL if fields are to be interpreted as RAW pointers instead of relative offsets
static inline VirtualFile create_virtual_file(
    const char* name,
    uint8_t  file_flags,
    uint16_t vfile_type,
    uint64_t data_size,
    uint64_t field_count,
    const void* fields_base_ptr,
    const uint64_t* fields
){
    return (const VirtualFile){
        .validated          = 1,
        .name               = name,
        .file_flags         = file_flags,
        .vfile_type         = vfile_type,
        .file_data_size     = data_size,
        .field_count        = field_count,
        .fields             = (uint64_t*) fields,
        .data               = (void*) fields_base_ptr,
    };
}

static inline int _vfopen_all_fields_found(const char** required_fields, const char** optional_fields){
    if(required_fields || optional_fields)     return 0;
    if(required_fields) if(required_fields[0]) return 0;
    if(optional_fields) if(optional_fields[0]) return 0;
    return 1;
}

int vfopen(VirtualFile* vfile, const char* path, const char** required_fields, const char** optional_fields){

    VIRTUAL_DEBUG_LOG("loading vfile '%s' to %p\n", path, vfile);

    if(!vfile){
        fprintf(stderr, "[INTERNAL ERROR] " __FILE__ "%i:0: missing output vfile\n", __LINE__);
        return 1;
    }

    vfile->validated = 0;

    vfile->name = NULL;

    int err = 0;

    char magic_number[5];
    magic_number[4] = '\0';

    Mc_stream_t stream = mc_create_stream(1024, 8);

    size_t meta_data_block = 0;

    FILE* f = fopen(path, "rb");

    if(!f){
        DEFER_ERROR("could not open file\n");
    }

    for(int i = 0; i < sizeof(VIRTUAL_FILE_MAGIC_NUMBER); i+=1){
        const int c = fgetc(f);
        if(c == EOF){
            DEFER_ERROR("could not read magic number\n");
        }
        magic_number[i] = (char) c;
    }

    VIRTUAL_DEBUG_LOG("reading header data\n");

    if(!mc_compare_str(magic_number, VIRTUAL_FILE_MAGIC_NUMBER, 0)){
        DEFER_ERROR("magic number '%s' does not match expected '%s'\n", magic_number, VIRTUAL_FILE_MAGIC_NUMBER);
    }
    vfile->file_flags = 0;
    for(int i = 0; i < sizeof(vfile->file_flags); i+=1){
        const int c = fgetc(f);
        if(c == EOF){
            DEFER_ERROR("failed to read internal file flags\n");
        }
        vfile->file_flags = (vfile->file_flags << 8) | (int8_t) c;
    }
    if(fread(&vfile->xflag, 1, sizeof(vfile->xflag), f) != sizeof(vfile->xflag)){
        DEFER_ERROR("failed to read xflag\n");
    }
    if(fread(&vfile->vfile_type, 1, sizeof(vfile->vfile_type) , f) != sizeof(vfile->vfile_type)){
        DEFER_ERROR("failed to read virtual file type\n");
    }
    if(fread(&vfile->field_count, 1 , sizeof(vfile->field_count), f) != sizeof(vfile->field_count)){
        DEFER_ERROR("failed to read field count\n");
    }
    if(fread(&vfile->file_data_size, 1 , sizeof(vfile->file_data_size), f) != sizeof(vfile->file_data_size)){
        DEFER_ERROR("failed to read meta data size\n");
    }

    VIRTUAL_DEBUG_LOG("accounting for endianess inconsistancies\n");

    if((vfile->file_flags & VIRTUAL_FILE_INTERNAL_FLAG_IS_LITTLE_ENDIAN) && !is_little_endian()){
        vfile->vfile_type        = mc_swap16(vfile->vfile_type);
        //vfile->field_count       = mc_swap64(vfile->field_count);
        vfile->file_data_size    = mc_swap64(vfile->file_data_size);
    }

    VIRTUAL_DEBUG_LOG(
        "done reading header data\n"
        "vfile_flags: %"PRIx8"  vxflag: %"PRIx8" vfile_type: %"PRIu16"  vfileds: %"PRIu64"  vdata_size: %"PRIu64"\n",
        vfile->file_flags, vfile->xflag, vfile->vfile_type, vfile->field_count, vfile->file_data_size
    );

    vfile->field_count = 0;

    for(size_t i = 0; i < vfile->file_data_size && !_vfopen_all_fields_found(required_fields, optional_fields); meta_data_block += 1){
        uint64_t size;
        uint64_t buff = 0;
        char*    id = (char*) &buff;

        VIRTUAL_DEBUG_LOG("reading field %zu at position %zu relative to data, and %li relative to file's start\n", meta_data_block, i, ftell(f));

        if(fread(&size, 1, sizeof(size), f) != sizeof(size))
            DEFER_ERROR("failed to read field's size\n");
        if((vfile->file_flags & VIRTUAL_FILE_INTERNAL_FLAG_IS_LITTLE_ENDIAN) && !is_little_endian())
            size = mc_swap64(size);
        if(size < sizeof(size) + 1){
            DEFER_ERROR(
                "meta data block %zu at position %zu has invalid size size %"PRIu64" < %zu\n",
                meta_data_block, i, size, (size_t) (sizeof(size) + sizeof(buff))
            );
        }
        
        int c = fgetc(f);
        int id_len = 0;
        for(; c && c != EOF && id_len + sizeof(size) < size && id_len < 8; id_len+=1){
            id[id_len] = (char) c;
            c = fgetc(f);
        }

        VIRTUAL_DEBUG_LOG("field: 0x%.*"PRIu64" '%.*s' of size %"PRIu64"\n", id_len, buff, id_len, id, size);

        if(c == EOF)
            DEFER_ERROR("file ended abrubtly at field %zu\n", i);
        if(id_len >= 8 || id_len == 0)
            DEFER_ERROR("field id should be between 1 and 7 bytes long, field %zu has 0x%.*"PRIx64" with %i instead\n", meta_data_block, id_len, buff, id_len);
        if(id[id_len])
            DEFER_ERROR("field id is not null terminated for field %zu 0x%.*"PRIx64" a.k.a '%.*s'\n", meta_data_block, id_len, buff, id_len, id);
        if(id_len + sizeof(size) >= size)
            DEFER_ERROR("field id '%.*s' %zu overflows field's size %"PRIu64"\n", id_len, id, id_len + sizeof(size), size);
        
        
        int field = query_field(id, required_fields);
        if(field < 0){
            VIRTUAL_DEBUG_LOG("field is not required\n");
            field = query_field(id, optional_fields);
        }
        if(field >= 0){
            const uintptr_t data = (uintptr_t) mc_stream_aligned(&stream, NULL, size, 8);
            VIRTUAL_DEBUG_LOG("streaming field data to %p at %"PRIu64" stream position\n", (void*) data, data - (uintptr_t) stream.data);
            *((uint64_t*) data) = size;
            memcpy((void*) (data + sizeof(size)), id, id_len + 1);
            const size_t read = fread((void*) (data + sizeof(size) + id_len + 1), 1, size - sizeof(size) - id_len - 1, f);
            if(read != size - sizeof(size) - id_len - 1){
                DEFER_ERROR("field %zu 0x%.*"PRIx64" was not read properly, expected to read %zu bytes, read %zu instead\n",
                    meta_data_block, id_len, buff, (size_t) (size - sizeof(size) - id_len - 1), read);
            }
            vfile->field_count += 1;
        }
        if(field < 0){
            VIRTUAL_DEBUG_LOG("field is neither optional nor required\n");
            if(fseek(f, size - sizeof(size) - id_len - 1, SEEK_CUR)){
                perror("fseek");
                VIRTUAL_DEBUG_ERR("tried to skip from %lu to fseek(f, %"PRIu64", SEEK_CUR);\n", ftell(f), (size_t) (size - sizeof(size) - id_len - 1));
                DEFER_ERROR("failed to skip field %zu 0x%.*"PRIx64", '%.*s'\n", meta_data_block, id_len, buff, id_len, id);
            }
        }
        
        i += size;
    }

    VIRTUAL_DEBUG_LOG("finninshed reading file's fields\n");

    if(required_fields){
        if(required_fields[0])
            DEFER_ERROR("missing required field '%s'\n", required_fields[0]);
    }

    if(vfile->field_count){
        VIRTUAL_DEBUG_LOG("allocating fields\n");
        vfile->fields = mc_stream_aligned(
            &stream, NULL,
            vfile->field_count * sizeof(vfile->fields[0]),
            sizeof(vfile->fields[0])
        );

        const uintptr_t data = (uintptr_t) stream.data;

        vfile->fields[0] = 0;

        for(uint64_t i = 1; i < vfile->field_count; i+=1){
            const uint64_t size = *(uint64_t*) (data + vfile->fields[i - 1]);
            vfile->fields[i] = vfile->fields[i - 1] + size;
            vfile->fields[i] += (8 - (vfile->fields[i] % 8)) % 8;
        }
    }
    else{
        VIRTUAL_DEBUG_LOG("no fields to allocate\n");
        vfile->fields = NULL;
    }

    VIRTUAL_DEBUG_LOG("validating file\n");
    vfile->data = stream.data;
    vfile->validated = 1;

    defer:
    VIRTUAL_DEBUG_LOG("cleaning up\n");
    if(f) fclose(f);
    if(err){
        mc_destroy_stream(stream);
        vfile->data = NULL;
        vfile->fields = NULL;
        VIRTUAL_DEBUG_ERR("could not load virtual file '%s'\n", path);
    }
    else VIRTUAL_DEBUG_LOG("loaded virtual file '%s' to %p\n", path, vfile);
    return err;
}

// \param name the fields name (up to 8 characters including null termination), if the name is already in data pass NULL
// \param data_size the data size in bytes
int add_virtual_file_field(VirtualFile* vfile, const char* name, uint64_t data_size, void* data){
    
    VIRTUAL_DEBUG_LOG("addind field '%s' to '%s'\n",name, vfile->name);

    if(!vfile) return 1;

    vfile->validated = 0;

    uint64_t name_len = 0;

    if(name){
        for(; name[name_len] && name_len < 8; name_len += 1);
        if(name_len == 8){
            return 1;
        }
    }

    Mc_stream_t stream = (Mc_stream_t){
        .data = vfile->data,
        .size = vfile->file_data_size,
        .capacity = vfile->file_data_size,
        .alignment = 8
    };

    uint64_t* fields = (uint64_t*) malloc((vfile->field_count + 1) * sizeof(vfile->fields[0]));

    for(uint64_t i = 0; i < vfile->field_count; i+=1){
        fields[i] = vfile->fields[i];
    }
    
    const uint64_t field_size = (name)?
        name_len + 1 + data_size + sizeof(field_size)
                        :
        name_len + data_size + sizeof(field_size);

    const uintptr_t field_ptr = (uintptr_t) mc_stream_aligned(&stream, &field_size, sizeof(field_size), 8);

    fields[vfile->field_count++] = (uint64_t) (field_ptr - (uintptr_t) (stream.data));
    mc_stream(&stream, name, name_len);

    const char null_termination_character = '\0';
    if(name) mc_stream(&stream, &null_termination_character, sizeof(null_termination_character));

    mc_stream(&stream, data, data_size);
    vfile->fields = (uint64_t*) mc_stream_aligned(&stream, fields, vfile->field_count * sizeof(fields[0]), sizeof(fields[0]));

    vfile->file_data_size += field_size;
    vfile->data = stream.data;

    free(fields);

    vfile->validated = 1;

    return 0;
}

void* get_virtual_file_field(const VirtualFile vfile, const char* field){

    VIRTUAL_DEBUG_LOG("quering '%s' field in virtual file '%s'\n", field, vfile.name);

    if(!vfile.validated){
        fprintf(stderr, "[ERROR] cannot get field from virtual file not properlly validated\n");
        return NULL;
    }
    if(!vfile.fields || !vfile.data){
        VIRTUAL_DEBUG_WARN("can not get field '%s' from virtual file as it has no valid fields\n", field);
        return NULL;
    }

    for(uint64_t i = 0; i < vfile.field_count; i+=1){
        const uintptr_t current_field = ((uintptr_t) vfile.data) + vfile.fields[i];
        const uint64_t  size = *((uint64_t*) current_field);
        VIRTUAL_DEBUG_LOG("comparing field '%.*s' at %"PRIu64" to '%s'\n",
            8, (char*) (current_field + sizeof(size)), vfile.fields[i], field);
        if(mc_compare_str((char*) (current_field + sizeof(size)), field, 0)){
            VIRTUAL_DEBUG_LOG("field found at %p\n", (const void*) current_field);
            return (void*) current_field;
        }
    }
    VIRTUAL_DEBUG_WARN("could not find field '%s' in virtual file\n", field);
    return NULL;
}

static inline void vfclose(VirtualFile vfile){
    if(vfile.data) virtual_free_aligned(vfile.data);
}


Inst* get_program_from_vfield(const void* _field, uint64_t* _program_size, uint64_t* entry_point){

    VIRTUAL_DEBUG_LOG("attempting to get program from field at %p\n", _field);

    if(_field == NULL){
        VIRTUAL_DEBUG_ERR("field == NULL cannot possibly be a program\n");
        return NULL;
    }

    const uintptr_t field = (uintptr_t) _field;
    uint64_t field_size = *(uint64_t*) _field;

    if(field_size < sizeof(field_size) + sizeof(VIRTUAL_FILE_PROGRAM_FIELD_NAME) + sizeof(*entry_point)){
        VIRTUAL_DEBUG_ERR("can not get program from field at %p ptr, field can not containg valid program\n", _field);
        return NULL;
    }
    if(0 == mc_compare_str((char*) (field + sizeof(field_size)), VIRTUAL_FILE_PROGRAM_FIELD_NAME, 0)){
        VIRTUAL_DEBUG_ERR("can not get program from field at %p ptr, not a program field\n", _field);
        return NULL;
    }
    
    if(entry_point) *entry_point = *(uint64_t*) (field + field_size - sizeof(*entry_point));

    if(_program_size) *_program_size = (field_size - (sizeof(field_size) + sizeof(VIRTUAL_FILE_PROGRAM_FIELD_NAME) + sizeof(*entry_point))) / sizeof(Inst);

    return (Inst*) (field + sizeof(field_size) + sizeof(VIRTUAL_FILE_PROGRAM_FIELD_NAME));
}

int vfsave(const VirtualFile vfile, const char* path){

    if(!vfile.validated){
        fprintf(
            stderr,
            "[ERROR] can't save vfile to '%s', vfile is not validaded, "
            "make sure it was generated and manipulated with the appropriate functions only\n",
            path
        );
        return 1;
    }

    int err = 0;

    VIRTUAL_DEBUG_LOG("saving vfile '%s' to '%s'\n", vfile.name, path);

    FILE* f = fopen(path, "wb");

    if(!f){
        fprintf(stderr, "[ERROR] could not open '%s'\n", path);
        return 1;
    }

    if(fwrite(VIRTUAL_FILE_MAGIC_NUMBER, 1, sizeof(VIRTUAL_FILE_MAGIC_NUMBER), f) != sizeof(VIRTUAL_FILE_MAGIC_NUMBER))
        DEFER_ERROR("failed to write magic_number\n");

    if(fwrite(&vfile.file_flags , 1, sizeof(vfile.file_flags), f) != sizeof(vfile.file_flags))
        DEFER_ERROR("failed to write internal_flags\n");
    if(fwrite(&vfile.xflag          , 1, sizeof(vfile.xflag)         , f) != sizeof(vfile.xflag))
        DEFER_ERROR("failed to write xflag\n");
    if(fwrite(&vfile.vfile_type     , 1, sizeof(vfile.vfile_type)    , f) != sizeof(vfile.vfile_type))
        DEFER_ERROR("failed to write virtual_file_type\n");
    if(fwrite(&vfile.field_count, 1 ,    sizeof(vfile.field_count)   , f) != sizeof(vfile.field_count))
        DEFER_ERROR("failed to write field_count\n");
    if(fwrite(&vfile.file_data_size , 1, sizeof(vfile.file_data_size), f) != sizeof(vfile.file_data_size))
        DEFER_ERROR("failed to write file_data_size\n");

    for(uint64_t i = 0; i < vfile.field_count; i+=1){
        const void* field = (void*) (((uintptr_t) vfile.data) + vfile.fields[i]);
        uint64_t field_size = *(uint64_t*) field;
        const char* const id = ((uint8_t*) field) + sizeof(field_size);
        VIRTUAL_DEBUG_LOG("writing field '%s' of size %"PRIu64"\n", id, field_size);
        if(fwrite(field, 1, field_size, f) != field_size)
            DEFER_ERROR("failed to write field %"PRIu64 "\n", i);
    }

    defer:
    VIRTUAL_DEBUG_LOG("cleaning up\n");
    if(f) fclose(f);
    return err;
}


#endif // =====================  END OF FILE VIRTUAL_FILES_H ===========================