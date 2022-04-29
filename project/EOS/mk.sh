echo "|===============================|"
echo "|           mk driver           |"
echo "|===============================|"
(cd driver; ./mk.sh)

echo "|===============================|"
echo "|             mk fs             |"
echo "|===============================|"
(cd fs; ./mk.sh)

echo "|===============================|"
echo "|           mk kernel           |"
echo "|===============================|"
(cd kernel; ./mk.sh)
cp -av kernel/t.bin kernel.bin

echo "|===============================|"
echo "|            mk USER            |"
echo "|===============================|"
rm -f sdimage
cp ../base_sdimage sdimage

echo "mount sdimage"
sudo mount -o loop sdimage /mnt

(cd USER; ./mk.sh)

echo "cp test files"
sudo cp -av test.sh /mnt/user/d3r3k
sudo chown 4:4 /mnt/user/d3r3k/test.sh
ls -l /mnt/user/d3r3k

echo "umount sdimage"
sudo umount /mnt

echo "|====== clean ======|"
echo clean driver
rm driver/*.o

echo clean fs
rm fs/*.o

echo clean kernel
rm kernel/*.o kernel/*.elf kernel/*.bin

echo clean USER
rm USER/*.o
rm USER/cmd/*.o USER/cmd/*.elf USER/cmd/*.bin
