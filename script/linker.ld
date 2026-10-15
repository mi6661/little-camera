ENTRY(Reset_Handler)


MEMORY {
    FLASH (rw) : ORIGIN = 0x08000000, LENGTH = 1024K
    CCM (rwx)  : ORIGIN = 0x10000000, LENGTH = 64K
    RAM1 (rwx) : ORIGIN = 0x20000000, LENGTH = 112K
    RAM2 (rwx) : ORIGIN = 0x2001C000, LENGTH = 16K
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
    } > RAM1 AT > FLASH
    _sidata = LOADADDR(.data);

    . = ALIGN(4);
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM1
    
    _estack = ORIGIN(RAM2) + LENGTH(RAM2);
}