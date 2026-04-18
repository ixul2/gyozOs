as -o bootentry.o  bootentry.S
ld -o bootentry.bin --oformat=binary -Ttext 0x7c00 -e main bootentry.o
qemu-system-x86_64 bootentry.bin
