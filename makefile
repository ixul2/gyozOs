all: bootable_disk
	qemu-system-x86_64 -drive format=raw,file=bootable_disk -no-reboot -drive file=external_disk.img,format=raw

bootable_disk: _obj/mkbootdisk kernel/_obj/all_kernel bootloader/_obj/all_boot
	_obj/mkbootdisk bootloader/_obj/all_boot kernel/_obj/all_kernel > bootable_disk

kernel_c_files = $(wildcard kernel/*.c)
kernel_obj_c_files = $(patsubst kernel/%.c,kernel/_obj/%.o,$(kernel_c_files))
kernel_as_files = $(wildcard kernel/*.s)
kernel_obj_as_files = $(patsubst kernel/%.s,kernel/_obj/assembly_%.o,$(kernel_as_files))
kernel/_obj/all_kernel : $(kernel_obj_c_files) $(kernel_obj_as_files)
	ld -o $@ -e kernel $^

kernel/_obj/assembly_%.o : kernel/%.s
	mkdir -p kernel/_obj
	as -o $@ $<

kernel/_obj/%.o : kernel/%.c
	mkdir -p kernel/_obj
	gcc -ffreestanding -c -o $@ $< 

bootloader/_obj/all_boot : bootloader/_obj/bootentry.o bootloader/_obj/bootloader.o
	ld -o $@ --oformat=binary -T bootloader/_build/boot.ld -e boot_start $^

bootloader/_obj/bootentry.o : bootloader/bootentry.S
	mkdir -p bootloader/_obj
	as -o $@ $<

bootloader/_obj/bootloader.o : bootloader/boot.c
	mkdir -p bootloader/_obj
	gcc -Os -ffreestanding -c -fno-inline -fomit-frame-pointer -o $@ $<

_obj/mkbootdisk : _build/mkbootdisk.c
	mkdir -p _obj
	gcc -o $@ $<

clean:

