#include "sml_vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long fs_filesize(FILE* file) 
{
    if (!file) {
        return -1L;
    }

    long pos = ftell(file);
    fseek(file, 0L, SEEK_END);
    long result = ftell(file);
    fseek(file, pos, SEEK_SET);
    return result;
}

void fs_strip_extension(char* name)
{
    char* p = name + strlen(name);

    while (name != p && *p != '.' && *p != '\\' && *p != '/') {
        --p;
    }

    if (p > name && *p == '.' && p[-1] != '\\' && p[-1] != '/') {
        *p = '\0';
    }
}

int fs_load_buffer(const char* name, void* buffer, long* psize)
{
    int result = 1;
    FILE* fp = fopen(name, "rb");
    if (!fp) {
        return 0;
    }

    long size = fs_filesize(fp);
    if (*psize == 0)
    {
        *psize = size;
    }
    else
    {
        if (*psize < size)
        {
            result = 0;
            goto error;
        }

        if (fread(buffer, sizeof(uint8_t), size, fp) != size)
        {
            result = 0;
        }

        *psize = size;
    }

error:
    fclose(fp);

    return result;
}

uint8_t* fs_load(const char* name, long* psize)
{
    long size = 0;
    fs_load_buffer(name, NULL, &size);
    uint8_t* buffer = (uint8_t*)calloc(size + 1, sizeof(uint8_t));
    if (!buffer) {
        return NULL;
    }

    if (!fs_load_buffer(name, buffer, &size))
    {
        free(buffer);
        buffer = NULL;
        size = 0;
    }

    if (psize) *psize = size;

    buffer[size] = '\0';

    return buffer;
}

void fs_free(uint8_t* buffer)
{
    free(buffer);
}