all : mtximage

mtximage : a.out
	dd if=a.out of=mtximage bs=1024 count=1 conv=notrunc
	ls -l mtximage

a.out : t.o bs.o
	ld86 -d bs.o t.o /usr/lib/bcc/libc.a
	ls -l a.out

t.o : t.c bs.s
	bcc -c -ansi t.c

bs.o : bs.s
	as86 -o bs.o bs.s

clean : getmtx
	rm -f a.out t.o bs.o

run:
	qemu-system-i386 -fda mtximage -no-fd-bootchk -serial mon:stdio -nographic

getmtx:
	wget -O mtximage eecs.wsu.edu/~cs460/samples/LAB1/mtximage
