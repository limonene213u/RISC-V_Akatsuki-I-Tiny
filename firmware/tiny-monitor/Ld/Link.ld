ENTRY(_start)

__stack_size = 2048;
PROVIDE(_stack_size = __stack_size);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 62K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .init :
    {
        _sinit = .;
        . = ALIGN(4);
        KEEP(*(SORT_NONE(.init)))
        . = ALIGN(4);
        _einit = .;
    } >FLASH

    .vector :
    {
        KEEP(*(.vector))
        . = ALIGN(64);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text .text.*)
        *(.rodata .rodata*)
        *(.gnu.linkonce.t.*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
    } >FLASH

    .fini :
    {
        KEEP(*(SORT_NONE(.fini)))
        . = ALIGN(4);
    } >FLASH

    PROVIDE(_etext = .);
    PROVIDE(_eitcm = .);

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
        PROVIDE_HIDDEN(__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP(*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .dtors))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } >FLASH

    .ctors :
    {
        KEEP(*crtbegin.o(.ctors))
        KEEP(*crtbegin?.o(.ctors))
        KEEP(*(EXCLUDE_FILE (*crtend.o *crtend?.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
    } >FLASH

    .dtors :
    {
        KEEP(*crtbegin.o(.dtors))
        KEEP(*crtbegin?.o(.dtors))
        KEEP(*(EXCLUDE_FILE (*crtend.o *crtend?.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
    } >FLASH

    .user_scratch ORIGIN(RAM) (NOLOAD) :
    {
        . = ALIGN(16);
        PROVIDE(__user_scratch_start = .);
        KEEP(*(.user_scratch))
        . = ORIGIN(RAM) + 0x1000;
        PROVIDE(__user_scratch_end = .);
    } >RAM

    .exec_buffer ORIGIN(RAM) + 0x1000 (NOLOAD) :
    {
        . = ALIGN(16);
        PROVIDE(__exec_buffer_start = .);
        KEEP(*(.exec_buffer))
        . = ORIGIN(RAM) + 0x1400;
        PROVIDE(__exec_buffer_end = .);
    } >RAM

    .data :
    {
        . = ALIGN(8);
        PROVIDE(_data_vma = .);
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.*)
        *(.sdata2 .sdata2.*)
        *(.srodata .srodata.*)
        . = ALIGN(4);
        PROVIDE(_edata = .);
    } >RAM AT>FLASH
    PROVIDE(_data_lma = LOADADDR(.data));

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        PROVIDE(_sbss = .);
        *(.sbss .sbss.*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        PROVIDE(_ebss = .);
    } >RAM

    PROVIDE(_end = _ebss);
    PROVIDE(end = .);

    .stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size (NOLOAD) :
    {
        PROVIDE(_heap_end = .);
        . = ALIGN(4);
        PROVIDE(_susrstack = .);
        . = . + __stack_size;
        PROVIDE(_eusrstack = .);
    } >RAM

    ASSERT(SIZEOF(.user_scratch) == 0x1000, "user scratch size changed")
    ASSERT(SIZEOF(.exec_buffer) == 0x400, "execution buffer size changed")
    ASSERT(_ebss <= _heap_end, "RAM overflow: monitor data overlaps stack")
}
