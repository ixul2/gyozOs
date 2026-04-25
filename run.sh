as -o bootentry.o  bootentry.S
gcc -ffreestanding -c -fno-pie boot.c -o boot.o
ld -o bootentry.bin --oformat=binary -T boot.ld -e boot_start bootentry.o boot.o
gcc -o boot_maker mkbootdisk.c
./boot_maker bootentry.bin > bootable_disk
qemu-system-x86_64 -d in_asm,cpu,int,exec -D debug-log -drive format=raw,file=bootable_disk
