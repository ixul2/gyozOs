###############################################################################
# BOOT ENTRY POINT  (identity‑maps first 2 MiB only)
###############################################################################

.code16

.set SEGSEL_BOOT_CODE, 0x8

.globl boot_start
boot_start:
        .code16
        cli
        cld
        movw    $boot_start, %sp

notify_bios64:
        movw    $0xEC00, %ax
        movw    $2, %dx
        int     $0x15

        .set INITIAL_PT,0x8000
        .set PTE_P,1
        .set PTE_W,2
        .set PTE_U,4
        .set PTE_PS,128

init_boot_pagetable:
        # clear memory for boot page table (L4 @ 0x1000, PDPT @ 0x2000, PD @ 0x3000)
        .set BOOT_PAGETABLE,0x1000
        movl    $BOOT_PAGETABLE, %edi
        xorl    %eax, %eax
        movl    $(0x3000 >> 2), %ecx        # 3 pages = 0x3000 bytes
        rep stosl

        # --- L4 table at 0x1000 ---
        # Entry 0  -> identity PDPT at 0x2000
        movl    $BOOT_PAGETABLE, %edi
        movl    $0x2000 + PTE_P + PTE_W, %ecx   # ecx = 0x2003
        movl    %ecx, (%edi)                    # L4[0] = 0x2003

        # All other L4 entries remain 0 (no high‑canonical mappings)

        # --- Identity PDPT at 0x2000 ---
        # Entry 0  -> PD at 0x3000
        movl    $0x2000, %edi
        movl    $0x3000 + PTE_P + PTE_W, %ecx   # ecx = 0x3003
        movl    %ecx, (%edi)                    # PDPT[0] = 0x3003

        # --- Identity PD at 0x3000 ---
        # Entry 0  -> 2 MiB page at physical 0x00000000
        movl    $0x3000, %edi
        movl    $(PTE_P + PTE_W + PTE_PS), %eax   # eax = 0x83
        movl    %eax, (%edi)                     # PD[0] = 0x83 (phys 0, 2 MiB)

        .set CR0_PE,0x1
        .set CR0_WP,0x10000
        .set CR0_PG,0x80000000
        .set CR4_PSE,0x10
        .set CR4_PAE,0x20
        .set MSR_IA32_EFER,0xC0000080
        .set IA32_EFER_SCE,1
        .set IA32_EFER_LME,0x100
        .set IA32_EFER_NXE,0x800

real_to_prot:
        movl    %cr4, %eax
        orl     $(CR4_PSE | CR4_PAE), %eax
        movl    %eax, %cr4
        movl    $BOOT_PAGETABLE, %eax       # CR3 = 0x1000
        movl    %eax, %cr3

        movl    $MSR_IA32_EFER, %ecx
        rdmsr
        orl     $(IA32_EFER_LME | IA32_EFER_SCE | IA32_EFER_NXE), %eax
        wrmsr

        movl    %cr0, %eax
        orl     $(CR0_PE | CR0_WP | CR0_PG), %eax
        movl    %eax, %cr0

        lgdt    gdtdesc + 6

        # Jump to boot() (still identity‑mapped, safe within the 2 MiB window)
        ljmp $SEGSEL_BOOT_CODE, $boot

# GDT (unchanged)
        .code32
        .p2align 3
gdt:    .word 0, 0, 0, 0
        .word 0, 0
        .byte 0, 0x9A, 0x20, 0
gdtdesc:
        .word 0, 0, 0
        .word 0x0f
        .long gdt
        .long 0