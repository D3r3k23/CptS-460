AS=as -Iinclude
LD=ld
CC=gcc

CPP=gcc -E -nostdinc
CFLAGS=-W -nostdlib -Wno-long-long -I include -fomit-frame-pointer

FS_OBJS= fs.o buffer.o util.o mount_root.o alloc_dealloc.o \
	mkdir_creat.o cd_pwd.o rmdir.o link_unlink.o stat.o touch.o \
	open_close.o read.o write.o dev.o mount_umount.o
.s.o:
	${AS} -a $< -o $*.o > $*.map

fsobj:	${FS_OBJS}

clean:
	rm -f fs *.o
