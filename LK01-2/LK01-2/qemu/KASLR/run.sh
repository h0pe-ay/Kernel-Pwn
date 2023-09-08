#!/bin/sh
qemu-system-x86_64 \
    -m 64M \
    -nographic \
    -kernel bzImage \
    -append "console=ttyS0 loglevel=3 oops=panic panic=-1 nosmap nosemp nopti kaslr" \
    -no-reboot \
    -cpu qemu64 \
    -smp 1 \
    -monitor /dev/null \
    -initrd initramfs.cpio.gz\
    -net nic,model=virtio \
    -net user \
    -s
