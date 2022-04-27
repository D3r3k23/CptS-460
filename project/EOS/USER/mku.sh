CMD=$1

echo "compile $CMD.c"
arm-none-eabi-gcc -w -c -I . -mcpu=arm926ej-s -o cmd/$CMD.o cmd/$CMD.c

echo "link $CMD"
arm-none-eabi-ld -T u.ld us.o cmd/$CMD.o -Ttext=0x80000000 -o cmd/$CMD.elf
arm-none-eabi-objcopy -O binary cmd/$CMD.elf cmd/$CMD.bin
