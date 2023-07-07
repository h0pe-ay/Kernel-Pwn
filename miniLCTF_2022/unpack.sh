mkdir rootfs
cd rootfs
cp ../rootfs.cpio .
cpio -idm < ./rootfs.cpio
rm rootfs.cpio
