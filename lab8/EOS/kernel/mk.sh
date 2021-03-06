echo "assemble ts.s"
arm-none-eabi-as -mcpu=arm926ej-s ts.s -o ts.o

for SRC in exec fork fe except kernel load mes pipe queue signal string syscall svc t thread wait
do
   echo "compile $SRC.c"
   arm-none-eabi-gcc -w -c -mcpu=arm926ej-s $SRC.c -o $SRC.o
done

echo "link kernel"
arm-none-eabi-ld -T t.ld *.o ../driver/*.o ../fs/*.o -Ttext=0x10000 -o t.elf
arm-none-eabi-objcopy -O binary t.elf t.bin
