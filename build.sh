#!/bin/sh

mkdir -p ./bin/linux

gcc src/sml_main.c src/sml_runtime.c src/sml_filesystem.c src/sml_c64_kernal.c src/sml_lexer.c src/sml_assembler.c -o ./bin/linux/c64asm -DPLATFORM_LINUX -DTOOLBUILD -DPROGRAM_NAME=runtime
gcc src/sml_main.c src/sml_runtime.c src/sml_filesystem.c src/sml_c64_kernal.c -o ./bin/linux/runtime -DPLATFORM_LINUX -DPROGRAM_NAME=runtime -fstack-protector-all -Wl,-z,now -s
gcc src/sml_main.c src/sml_runtime.c src/sml_filesystem.c src/sml_c64_kernal.c -o ./bin/linux/runtime_fmtstr -DPLATFORM_LINUX -DPROGRAM_NAME=runtime -DIS_PWN_FMTSTR -no-pie -fstack-protector-all -s 

./bin/linux/c64asm --asm --out ./deploy/rev/public/program.prg ./src/programs/chal_flag_checker.s
./bin/linux/c64asm --asm --out ./deploy/ret2win/deploy/program.prg ./src/programs/chal_ret2win.s
./bin/linux/c64asm --asm --out ./deploy/shellcode/deploy/program.prg ./src/programs/chal_shellcode.s
./bin/linux/c64asm --asm --out ./deploy/shellcode/public/program.prg ./src/programs/chal_shellcode.s

cp ./bin/linux/runtime deploy/rev/public
cp ./bin/linux/runtime deploy/ret2win/public
cp ./bin/linux/runtime deploy/ret2win/deploy
cp ./bin/linux/runtime deploy/shellcode/public
cp ./bin/linux/runtime deploy/shellcode/deploy
cp ./bin/linux/runtime_fmtstr deploy/format/public/runtime
cp ./bin/linux/runtime_fmtstr deploy/format/deploy/runtime
