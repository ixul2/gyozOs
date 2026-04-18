as -o bootentry.o  bootentry.S
gcc -ffreestanding -c -no-pie boot.c -o boot.o
ld -o bootentry.bin --oformat=binary -Ttext 0x7c00 -e main bootentry.o
qemu-system-x86_64 bootentry.bin
