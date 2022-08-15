.global _start
.extern start

.section .data

result: .dword 0


.section .text
/*
sum:
        str lr, [sp, -16]!
        add x0, x0, x1
        ldr lr, [sp], 16
        ret
*/

_start:
.setup_stack:
        ldr x0, =STACK_END
        mov sp, x0

        bl start

.loop:
        b .loop
