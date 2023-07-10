mkdir initramfs
cd initramfs
cp ../initramfs.cpio .
cpio -idm < ./initramfs.cpio
rm initramfs.cpio
