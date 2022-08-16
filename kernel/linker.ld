ENTRY(_start)

SECTIONS
{
        . = 0x40000000;
        EXCEPTION_TABLE = .;
        .exceptions_table : {_exceptions.o(.text)}
        . = 0x41000000;
        GIC_SLAB_START = .;
        . = 0x41100000;
        GIC_SLAB_END = .;



        . = 0x50000000;
        .text : {*(.text)}
        .data : {*(.data)}
        .bss : {*(.bss)}

        . = ALIGN(16);
        STACK_START = .;
        . += 0x10000;
        STACK_END = .;

        . = ALIGN(16);
        USERSTACK_START = .;
        . += 0x10000;
        USERSTACK_END = .;
}
