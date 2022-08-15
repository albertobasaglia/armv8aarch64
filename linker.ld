ENTRY(_start)

SECTIONS
{
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
