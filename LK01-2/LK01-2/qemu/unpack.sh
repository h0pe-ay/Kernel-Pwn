mkdir initramfs
cd initramfs
cp ../rootfs.cpio .
cpio -idm < ./rootfs.cpio
rm rootfs.cpio
