AS=arm-none-eabi-as -mcpu=arm926ej-s
LD=arm-none-eabi-ld
CC=arm-none-eabi-gcc -mcpu=arm926ej-s

KERNEL_OBJS= t.o ts.o queue.o fork.o exec.o wait.o io.o syscall.o loader.o pipe.o
U_ADDR=0x00000
K_ADDR=0x80100000


all: mtx_kernel #user

# for all .s files: as fname.s as fname.o; redirect listing to fname.map
.s.o:
	${AS} -a $< -o $*.o > $*.map

mtx_kernel: $(KERNEL_OBJS)
	cd ../fs && $(MAKE)
	cd ../driver && $(MAKE)

	${LD} --oformat binary -Map k.map -N -e pm_entry -Ttext ${K_ADDR} -o $@ \
	${KERNEL_OBJS} ../driver/*.o ../fs/*.o  ../mtxlib

	@wc -c mtx_kernel

clean:
	rm -f *.map *.o
