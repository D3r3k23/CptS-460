echo "assemble us.s"
arm-none-eabi-as -mcpu=arm926ej-s us.s -o us.o

for CMD in test init login sh cat cp grep ls
do
    echo "====== mk $CMD ======"
    ./mku.sh $CMD
    sudo cp -av $CMD.bin /mnt/bin/$CMD
done
