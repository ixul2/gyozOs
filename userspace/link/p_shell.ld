ENTRY(process_main)

SECTIONS {
    . = 0x140000;

    .text : { *(.text) }

    .rodata : { *(.rodata) }

    .data : { *(.data) }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}