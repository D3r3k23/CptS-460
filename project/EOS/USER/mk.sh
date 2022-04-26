echo "assemble us.s"
arm-none-eabi-as -mcpu=arm926ej-s us.s -o us.o

for CMD in test init login sh cat cp grep l2u ls mkdir more rm
do
    echo "====== mk $CMD ======"
    ./mku.sh $CMD
    sudo cp -av $CMD.bin /mnt/bin/$CMD
    sudo chown 0:0 /mnt/bin/$CMD
done

echo "installed user programs on sdimage:"
ls -l /mnt/bin | grep derek
