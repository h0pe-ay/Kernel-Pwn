#!/bin/sh
qemu-system-x86_64 \
    -m 64M \
    -nographic \
    -kernel bzImage \
    -append "console=ttyS0 loglevel=3 oops=panic panic=-1 pti=on kaslr" \
    -no-reboot \
    -cpu qemu64,+smap,+smep \
    -smp 2 \
    -monitor /dev/null \
    -initrd initramfs.cpio.gz \
    -net nic,model=virtio \
    -net user \
    -s
