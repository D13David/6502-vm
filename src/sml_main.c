#include "sml_vm.h"

#include <string.h>
#include <stdlib.h>

#define stringify(s) stringify1(s)
#define stringify1(s) #s

//#define DEBUG_RUN_FROM_CODE 1

#if !TOOLBUILD
#undef DEBUG_RUN_FROM_CODE
#endif

int main(int argc, const char** argv)
{
    int assemble_file = 0;
    int run = 0;
    const char* input = NULL;
    char output[FILENAME_MAX];

    memset(output, 0, sizeof(output));

#ifndef DEBUG_RUN_FROM_CODE
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "--asm") || !strcmp(argv[i], "-a"))
            {
                assemble_file = 1;
            }
            else if (!strcmp(argv[i], "--out") || !strcmp(argv[i], "-o"))
            {
                if (argc > i + 1) {
                    strncpy(output, argv[i + 1], FILENAME_MAX);
                    ++i;
                }
            }
        }
        else
        {
            input = argv[i];
            run = 1;
        }
    }

    // we don't want to run in assembler mode
    if (assemble_file) {
        run = 0;
    }
#else
    assemble_file = 1;
    run = 1;
    input = "src/programs/chal_ret2win.s";
#endif
    if (input == NULL)
    {
        printf("usage: %s [options] program\n"
                      "  options:\n"
                      "    -a, --asm assemble source\n"
                      "    -o, --output set output name for assembler", stringify(PROGRAM_NAME));
        return 0;
    }

    long programSize;
    uint8_t* program = fs_load(input, &programSize);

#if TOOLBUILD
    if (assemble_file) 
    {
        if (strlen(output) == 0)
        {
            strncpy(output, input, FILENAME_MAX);
            fs_strip_extension(output);
            strcat(output, ".prg");
        }

        if (compile(program, programSize, output) == 0) 
        {
            printf("compile failed");
            return -1;
        }
        else
        {
            printf("successfully compiled %s, %s\n", input, output);
        }
    }
#endif
    if (run)
    {
#ifdef _DEBUG
        fs_free(program);
        program = fs_load(output, &programSize);
#endif
        vm_init();
        vm_load(program, programSize);
        vm_run();
        vm_shutdown();
    }

    fs_free(program);
}