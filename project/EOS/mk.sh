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

echo "sdimage /bin:"
ls -l /mnt/bin

echo "sdimage /user/d3r3k:"
ls -l /mnt/user/d3r3k

echo "umount sdimage"
sudo umount /mnt

echo "|====== clean ======|"
for DIR in driver fs
do
    echo "clean $DIR"
    rm $DIR/*.o
done
for DIR in kernel USER
do
    echo "clean $DIR"
    rm $DIR/*.o $DIR/*.elf $DIR/*.bin
done
