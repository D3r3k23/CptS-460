CMD=$1

echo "compile $CMD.c"
arm-none-eabi-gcc -w -c -mcpu=arm926ej-s -o $CMD.o $CMD.c

echo "link $CMD"
arm-none-eabi-ld -T u.ld us.o $CMD.o -Ttext=0x80000000 -o $CMD.elf
arm-none-eabi-objcopy -O binary $CMD.elf $CMD.bin
