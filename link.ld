ENTRY(Reset_Handler)

MEMORY {
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}


SECTIONS {
    .text : {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
    } > FLASH

    _sidata = LOADADDR(.data);

    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {
        . = ALIGN(4);
        _sbss = .;
        PROVIDE(__bss_start__ = _sbss);
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        PROVIDE(__bss_end__ = _ebss);
    } > RAM

    . = ALIGN(4);
    end = .;

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}