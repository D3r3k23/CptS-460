for SRC in disk sdc kbd pv timer uart vid
do
   echo "compile $SRC.c"
   arm-none-eabi-gcc -w -c -mcpu=arm926ej-s $SRC.c -o $SRC.o
done
