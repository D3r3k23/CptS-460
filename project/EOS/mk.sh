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
cp ../original_sdimage sdimage
(cd USER; ./mk.sh)

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
