for SRC in alloc_dealloc buffer cd_pwd dev fs link_unlink mkdir_creat \
   mount_root mount_umount open_close read rmdir stat touch util write
do
   echo "compile $SRC.c"
   arm-none-eabi-gcc -w -c -mcpu=arm926ej-s $SRC.c -o $SRC.o
done
