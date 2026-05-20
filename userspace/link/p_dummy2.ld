ENTRY(process_main)

SECTIONS {
    . = 0x1C0000;

    .text : { *(.text) }

    .rodata : { *(.rodata) }

    .data : { *(.data) }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}