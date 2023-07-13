mkdir core
cd core
cp ../core.cpio .
cpio -idm < ./core.cpio
rm core.cpio
