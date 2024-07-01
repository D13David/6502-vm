#include "sml_vm.h"

#include <stdio.h>
#include <string.h>

#define IDX_IO_STATUS_FLAG             0x90
#define IDX_CURRENT_INPUT_DEVICE_NUM   0x99
#define IDX_CURRENT_OUTPUT_DEVICE_NUM  0x9A
#define IDX_FILE_NAME_LENGTH           0xB7
#define IDX_LOGICAL_FILE_NUMBER        0xB8
#define IDX_FILE_SECONDARY_ADDRESS     0xB9
#define IDX_DEVICE_NUMBER              0xBA
#define IDX_FILE_NAME                  0xBB

#define DEVICE_NUM_KEYBOARD             0
#define DEVICE_NUM_DATASETTE            1
#define DEVICE_NUM_MODEM                2
#define DEVICE_NUM_SCREEN               3
#define DEVICE_NUM_PRINTER_1            4 // 4 - 5 printer
#define DEVICE_NUM_DISK_DRIVE_1         8 // 8 - 15 disk drive

#define INVALID_DEVICE_NUM              99

#define MAX_LOGICAL_FILES               256

typedef struct file
{
    uint8_t deviceNum;
    void* data;
} file_t;

file_t GFileTable[MAX_LOGICAL_FILES];

void set_ac(uint8_t value)
{
    vm_context.regs.AC = value;
    set_flag(FLAG_N, vm_context.regs.AC & 0x80);
    set_flag(FLAG_Z, !vm_context.regs.AC);
}

void __func_kernal_load()
{
    set_flag(FLAG_C, 0);

    long size = 0xffff;
    uint16_t address = (vm_context.memory[IDX_FILE_NAME + 1] << 8) | vm_context.memory[IDX_FILE_NAME];
    char name[FILENAME_MAX] = { '\0' };
    strncat(name, &vm_context.memory[address], vm_context.memory[IDX_FILE_NAME_LENGTH]);

    if (strlen(name) == 0)
    {
        set_ac(8); // no name was specified for a serial load
        set_flag(FLAG_C, 1);
        return;
    }

    uint8_t deviceNumber = vm_context.memory[IDX_DEVICE_NUMBER];
    if (deviceNumber < 8)
    {
        set_ac(9); // an illegal device number was specified
        set_flag(FLAG_C, 1);
        return;
    }

    // not supported... 
    int absoluteLoad = vm_context.memory[IDX_FILE_SECONDARY_ADDRESS] & 0x1;

    if (vm_context.memory[IDX_DEVICE_NUMBER] >= 8)
    {
        if (vm_context.regs.AC == 0)
        {
            address = (vm_context.regs.Y << 8) | vm_context.regs.X;
            if (!fs_load_buffer(name, &vm_context.memory[address], &size))
            {
                set_ac(4); // file was not found
                set_flag(FLAG_C, 1);
            }
        }
        else
        {
            // verify is not implemented...
        }
    }
}

void __func_kernal_setlfs()
{
    vm_context.memory[IDX_LOGICAL_FILE_NUMBER] = vm_context.regs.AC;
    vm_context.memory[IDX_DEVICE_NUMBER] = vm_context.regs.X;
    vm_context.memory[IDX_FILE_SECONDARY_ADDRESS] = vm_context.regs.Y;
}

void __func_kernal_setnam()
{
    vm_context.memory[IDX_FILE_NAME_LENGTH] = vm_context.regs.AC;
    vm_context.memory[IDX_FILE_NAME + 0] = vm_context.regs.X;
    vm_context.memory[IDX_FILE_NAME + 1] = vm_context.regs.Y;
}

void __func_kernal_chrin()
{
    if (vm_context.memory[IDX_CURRENT_INPUT_DEVICE_NUM] == DEVICE_NUM_KEYBOARD) {
        set_ac(getc(stdin));
    }
    else
    {
        uint8_t logicalFileNum = vm_context.memory[IDX_LOGICAL_FILE_NUMBER];
        if (GFileTable[logicalFileNum].deviceNum != INVALID_DEVICE_NUM) 
        {
            FILE* fp = (FILE*)GFileTable[logicalFileNum].data;
            fread(&vm_context.regs.AC, sizeof(uint8_t), 1, fp);

            if (feof(fp)) {
                vm_context.memory[IDX_IO_STATUS_FLAG] |= 0x40;
            }
            else {
                vm_context.memory[IDX_IO_STATUS_FLAG] &= ~0x40;
            }
        }
    }
}

void __func_kernal_chrout()
{
    if (vm_context.memory[IDX_CURRENT_OUTPUT_DEVICE_NUM] == DEVICE_NUM_SCREEN) {
        putchar(vm_context.regs.AC);
    }
    else
    {
        vm_panic(); // we don't support any output other than screen
    }
}

void __func_kernal_readst()
{
    set_ac(vm_context.memory[IDX_IO_STATUS_FLAG]);
}

void __func_kernal_open()
{
    uint8_t logicalFileNumber = vm_context.memory[IDX_LOGICAL_FILE_NUMBER];
    if (GFileTable[logicalFileNumber].deviceNum != INVALID_DEVICE_NUM) 
    {
        set_flag(FLAG_C, 1);
        set_ac(2); // a currently open file already uses the specified logical file number
        return;
    }

    uint8_t deviceNum = vm_context.memory[IDX_DEVICE_NUMBER];

    if (deviceNum >= DEVICE_NUM_DISK_DRIVE_1)
    {
        uint16_t address = (vm_context.memory[IDX_FILE_NAME + 1] << 8) | vm_context.memory[IDX_FILE_NAME];
        char name[FILENAME_MAX] = { '\0' };
        strncat(name, &vm_context.memory[address], vm_context.memory[IDX_FILE_NAME_LENGTH]);

        FILE* fp = fopen(name, "rb");
        if (!fp)
        {
            set_flag(FLAG_C, 1);
            set_ac(5); //specified device did not respond
        }
        else
        {
            set_flag(FLAG_C, 0);
            GFileTable[logicalFileNumber].data = fp;
        }
    }
    else
    {
        set_flag(FLAG_C, 0);
    }

    if ((vm_context.regs.SR & FLAG_C) == 0) {
        GFileTable[logicalFileNumber].deviceNum = deviceNum;
    }
}

void __func_kernal_close()
{
    uint8_t logicalFileNumber = vm_context.memory[IDX_LOGICAL_FILE_NUMBER];
    if (GFileTable[logicalFileNumber].deviceNum != INVALID_DEVICE_NUM)
    {
        if (GFileTable[logicalFileNumber].deviceNum >= DEVICE_NUM_DISK_DRIVE_1)
        {
            fclose((FILE*)GFileTable[logicalFileNumber].data);
        }

        GFileTable[logicalFileNumber].deviceNum = INVALID_DEVICE_NUM;
        GFileTable[logicalFileNumber].data = NULL;
    }
}

void __func_kernal_chkin()
{
    set_flag(FLAG_C, 0);
    file_t* logicalFile = &GFileTable[vm_context.regs.X];
    if (logicalFile->deviceNum == INVALID_DEVICE_NUM)
    {
        set_flag(FLAG_C, 1);
        set_ac(3); //file was not open
        return;
    }

    int canReadFromDevice = (logicalFile->deviceNum >= DEVICE_NUM_KEYBOARD && logicalFile->deviceNum <= DEVICE_NUM_MODEM 
                             || logicalFile->deviceNum >= DEVICE_NUM_DISK_DRIVE_1);

    if (!canReadFromDevice) 
    {
        set_flag(FLAG_C, 1);
        set_ac(6); //file was not opened for input
        return;
    }

    vm_context.memory[IDX_CURRENT_INPUT_DEVICE_NUM] = logicalFile->deviceNum;
}

void __func_kernal_chkout()
{
    set_flag(FLAG_C, 0);
    file_t* logicalFile = &GFileTable[vm_context.regs.X];
    if (logicalFile->deviceNum == INVALID_DEVICE_NUM)
    {
        set_flag(FLAG_C, 1);
        set_ac(3); //file was not open
        return;
    }

    int canWriteToDevice = logicalFile->deviceNum >= DEVICE_NUM_DATASETTE;
    if (!canWriteToDevice)
    {
        set_flag(FLAG_C, 1);
        set_ac(7); //file was not opened for output
        return;
    }

    vm_context.memory[IDX_CURRENT_OUTPUT_DEVICE_NUM] = logicalFile->deviceNum;
}

void __func_kernal_clrchn()
{
    vm_context.memory[IDX_CURRENT_INPUT_DEVICE_NUM] = DEVICE_NUM_KEYBOARD;
    vm_context.memory[IDX_CURRENT_OUTPUT_DEVICE_NUM] = DEVICE_NUM_SCREEN;
}

void kernal_routines_call()
{
    switch (vm_context.regs.PC)
    {
    case 0xFFB7: /* READST */
        __func_kernal_readst();
        break;
    case 0xFFBA: /* SETLFS */
        __func_kernal_setlfs();
        break;
    case 0xFFBD: /* SETNAM */
        __func_kernal_setnam();
        break;
    case 0xFFC0: /* OPEN */
        __func_kernal_open();
        break;
    case 0xFFC3: /* CLOSE */
        __func_kernal_close();
        break;
    case 0xFFC6: /* CHKIN */
        __func_kernal_chkin();
        break;
    case 0xFFC9: /* CHKOUT */
        __func_kernal_chkout();
        break;
    case 0xFFCC: /* CLRCHN */
        __func_kernal_clrchn();
        break;
    case 0xFFCF: /* CHRIN */
        __func_kernal_chrin();
        break;
    case 0xFFD2: /* CHROUT */
        __func_kernal_chrout();
        break;
    case 0xFFD5: /* LOAD */
        __func_kernal_load();
        break;
    default:
        vm_panic();
    }
}

void kernal_init()
{
    for (int i = 0; i < MAX_LOGICAL_FILES; ++i)
    {
        GFileTable[i].deviceNum = INVALID_DEVICE_NUM;
        GFileTable[i].data = NULL;
    }

    vm_context.memory[IDX_IO_STATUS_FLAG] = 0;
    vm_context.memory[IDX_CURRENT_INPUT_DEVICE_NUM] = DEVICE_NUM_KEYBOARD;
    vm_context.memory[IDX_CURRENT_OUTPUT_DEVICE_NUM] = DEVICE_NUM_SCREEN;
}