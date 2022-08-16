.section .data
message: .asciz "Ciao!\n"

.section .text

//current_level_el0
// SYNC
.balign 0x80
        b .
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
        b .
// SERROR
.balign 0x80
        b .

//current_level_elx
// SYNC
.balign 0x80
        b .
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
        ldr x0, =message
        bl put_string
        b .
// SERROR
.balign 0x80
        b .

//lower_aarch64
// SYNC
.balign 0x80
        b .
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
        b .
// SERROR
.balign 0x80
        b .

//lower_aarch32
// SYNC
.balign 0x80
        b .
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
        b .
// SERROR
.balign 0x80
        b .
