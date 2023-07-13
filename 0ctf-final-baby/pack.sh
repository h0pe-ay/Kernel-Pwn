gcc -o exploit -static $1 -pthread
mv ./exploit ./core
cd core
find . -print0 \
| cpio --null -ov --format=newc \
| gzip -9 > core.cpio.gz
mv ./core.cpio.gz ../
