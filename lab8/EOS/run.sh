KERNEL="kernel.bin"

qemu-system-arm -M versatilepb -m 256M -sd sdimage -kernel $KERNEL -serial mon:stdio -serial /dev/pts/1
