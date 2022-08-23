ENTRY(_start)

SECTIONS {
        . = 0x80000000;

        .text : {*.o(.text)}

        .data : {*.o(.data)}
}
