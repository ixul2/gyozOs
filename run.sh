as -o obj/bootentry.o  bootentry.S
gcc -ffreestanding -c -fno-pie boot.c -o obj/boot.o
ld -o obj/bootentry.bin --oformat=binary -T build/boot.ld -e boot_start obj/bootentry.o obj/boot.o
gcc -o obj/boot_maker build/mkbootdisk.c
obj/boot_maker obj/bootentry.bin > bootable_disk
#qemu-system-x86_64 -d in_asm,cpu,int,exec -D debug-log -drive format=raw,file=bootable_disk
qemu-system-x86_64 -drive format=raw,file=bootable_disk
