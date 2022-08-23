ENTRY(_start)

SECTIONS
{
        . = 0x40000000;
        EXCEPTION_TABLE = .;
        .exceptions_table : {_exceptions.o(.text)}

        . = 0x40100000;
        HEAP_START = .;



        . = 0x50000000;
        .text : {*(.text)}
        .data : {*(.data)}
        .bss : {*(.bss)}

        . = ALIGN(16);
        STACK_START = .;
        . += 0x10000;
        STACK_END = .;

}
