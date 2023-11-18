#pragma once

#include <stdint.h>
#include <stdio.h>

#include "sml_6502_public.h"

void vm_init();
void vm_shutdown();
int vm_load(const uint8_t* program, long size);
void vm_run();

void kernal_init();
void kernal_routines_call();

long fs_filesize(FILE* file);
void fs_strip_extension(char* name);
uint8_t* fs_load(const char* name, long* psize);
int fs_load_buffer(const char* name, void* buffer, long* psize);
void fs_free(uint8_t* buffer);

#if TOOLBUILD

int compile(const char* program, uint32_t size, const char* out_name);

#endif