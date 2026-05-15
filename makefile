all: bootable_disk run

run:
	qemu-system-x86_64 \
		-drive format=raw,file=bootable_disk \
		-no-reboot \
		-drive file=external_disk.img,format=raw \
		-serial stdio \
		-d int,cpu_reset,guest_errors


# =========================================================
# Final bootable image
# =========================================================

bootable_disk: _obj/mkbootdisk \
               bootloader/_obj/all_boot \
               kernel/_obj/all_kernel \
               userspace/_obj/p_shell.bin
	_obj/mkbootdisk \
		bootloader/_obj/all_boot \
		kernel/_obj/all_kernel \
		userspace/_obj/p_shell.bin \
		> bootable_disk


# =========================================================
# Kernel
# =========================================================

kernel_c_files := $(wildcard kernel/*.c)
kernel_obj_c_files := $(patsubst kernel/%.c,kernel/_obj/%.o,$(kernel_c_files))

kernel_as_files := $(wildcard kernel/*.s)
kernel_obj_as_files := $(patsubst kernel/%.s,kernel/_obj/assembly_%.o,$(kernel_as_files))


kernel/_obj/all_kernel: $(kernel_obj_c_files) $(kernel_obj_as_files) userspace/_obj/p_shell_embedded.o
	ld -T kernel/link/kernel.ld -e kernel -o $@ $^


kernel/_obj/assembly_%.o: kernel/%.s
	mkdir -p kernel/_obj
	as -o $@ $<


kernel/_obj/%.o: kernel/%.c kernel/%.h
	mkdir -p kernel/_obj
	gcc \
		-mno-red-zone \
		-mno-mmx \
		-mno-sse \
		-ffreestanding \
		-fno-pie \
		-fno-pic \
		-m64 \
		-c \
		-o $@ $<


# =========================================================
# Bootloader
# =========================================================

bootloader/_obj/all_boot: \
	bootloader/_obj/bootentry.o \
	bootloader/_obj/bootloader.o
	ld \
		--oformat=binary \
		-T bootloader/_build/boot.ld \
		-e boot_start \
		-o $@ \
		$^


bootloader/_obj/bootentry.o: bootloader/bootentry.S
	mkdir -p bootloader/_obj
	as -o $@ $<


bootloader/_obj/bootloader.o: bootloader/boot.c
	mkdir -p bootloader/_obj
	gcc \
		-Os \
		-ffreestanding \
		-fno-inline \
		-fomit-frame-pointer \
		-c \
		-o $@ \
		$<


# =========================================================
# Userspace : p_shell
# =========================================================

userspace/_obj/p_shell.o: userspace/p_shell.c
	mkdir -p userspace/_obj
	gcc \
		-ffreestanding \
		-fno-pie \
		-fno-pic \
		-m64 \
		-c \
		-o $@ $<


userspace/_obj/p_shell.bin: userspace/_obj/p_shell.o
	ld \
		-T userspace/link/p_shell.ld \
		--oformat=binary \
		-o $@ \
		$^


# =========================================================
# Embed p_shell.bin into kernel
# =========================================================

userspace/_obj/p_shell_embedded.o: userspace/_obj/p_shell.bin
	ld \
		-r \
		-b binary \
		-o $@ \
		$<

# =========================================================
# Tools
# =========================================================

_obj/mkbootdisk: _build/mkbootdisk.c
	mkdir -p _obj
	gcc -o $@ $<


# =========================================================
# Clean
# =========================================================

clean:
	rm -rf \
		kernel/_obj \
		bootloader/_obj \
		userspace/_obj \
		_obj \
		bootable_disk