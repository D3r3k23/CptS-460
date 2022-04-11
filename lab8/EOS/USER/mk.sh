echo "assemble us.s"
arm-none-eabi-as -mcpu=arm926ej-s us.s -o us.o

echo "mount sdimage"
sudo mount -o loop ../sdimage /mnt

for CMD in test fe
do
    echo "====== mk $CMD ======"
    ./mku.sh $CMD
    sudo cp -av $CMD.bin /mnt/bin/$CMD
done

echo "sdimage /bin:"
ls -l /mnt/bin

echo "umount sdimage"
sudo umount /mnt
