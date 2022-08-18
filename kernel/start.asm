.global _start
.extern start

.section .data
.section .text

_start:
.setup_stack:
        ldr x0, =STACK_END
        mov sp, x0

        // enable fpu
        mrs x0, CPACR_EL1
        orr x0, x0, (0b11 << 20)
        msr CPACR_EL1, x0

        bl start

.loop:
        b .loop
