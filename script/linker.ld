ENTRY(Reset_Handler)


MEMORY {
    FLASH (rw) : ORIGIN = 0x08000000, LENGTH = 1024K
    CCM (rwx)  : ORIGIN = 0x10000000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS {
    .text : {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
    } > FLASH
    
    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);

    . = ALIGN(4);
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
    
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}