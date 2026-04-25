as -o bootentry.o  bootentry.S
gcc -ffreestanding -c -fno-pie boot.c -o boot.o
ld -o bootentry.bin --oformat=binary -Ttext 0x7c00 -e boot_start bootentry.o boot.o
qemu-system-x86_64 -d in_asm,cpu,int,exec -D debug-log -drive format=raw,file=bootentry.bin
