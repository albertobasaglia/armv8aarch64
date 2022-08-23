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
        bl exceptions_distributor
        eret
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
excbreak:
        bl exceptions_handle_fiq
        eret
// SERROR
.balign 0x80
        b .

//lower_aarch64
// SYNC
.balign 0x80
        // stack is FULL DESCENDING
        // save context
        stp x0, x1, [sp, #-16]!
        stp x2, x3, [sp, #-16]!
        stp x4, x5, [sp, #-16]!
        stp x6, x7, [sp, #-16]!
        stp x8, x9, [sp, #-16]!
        stp x10, x11, [sp, #-16]!
        stp x12, x13, [sp, #-16]!
        stp x14, x15, [sp, #-16]!
        stp x16, x17, [sp, #-16]!
        stp x18, x19, [sp, #-16]!
        stp x20, x21, [sp, #-16]!
        stp x22, x23, [sp, #-16]!
        stp x24, x25, [sp, #-16]!
        stp x26, x27, [sp, #-16]!
        stp x28, x29, [sp, #-16]!
        stp x30, xzr, [sp, #-16]!
        mov x0, sp
        bl exceptions_distributor
        // restore context
        ldp x30, xzr, [sp], #16
        ldp x28, x29, [sp], #16
        ldp x26, x27, [sp], #16
        ldp x24, x25, [sp], #16
        ldp x22, x23, [sp], #16
        ldp x20, x21, [sp], #16
        ldp x18, x19, [sp], #16
        ldp x16, x17, [sp], #16
        ldp x14, x15, [sp], #16
        ldp x12, x13, [sp], #16
        ldp x10, x11, [sp], #16
        ldp x8, x9, [sp], #16
        ldp x6, x7, [sp], #16
        ldp x4, x5, [sp], #16
        ldp x2, x3, [sp], #16
        ldp x0, x1, [sp], #16
        eret
// IRQ
.balign 0x80
        b .
// FIQ
.balign 0x80
        // stack is FULL DESCENDING
        // save context
        stp x0, x1, [sp, #-16]!
        stp x2, x3, [sp, #-16]!
        stp x4, x5, [sp, #-16]!
        stp x6, x7, [sp, #-16]!
        stp x8, x9, [sp, #-16]!
        stp x10, x11, [sp, #-16]!
        stp x12, x13, [sp, #-16]!
        stp x14, x15, [sp, #-16]!
        stp x16, x17, [sp, #-16]!
        stp x18, x19, [sp, #-16]!
        stp x20, x21, [sp, #-16]!
        stp x22, x23, [sp, #-16]!
        stp x24, x25, [sp, #-16]!
        stp x26, x27, [sp, #-16]!
        stp x28, x29, [sp, #-16]!
        stp x30, xzr, [sp, #-16]!
        mov x0, sp
        bl exceptions_handle_fiq
        // restore context
        ldp x30, xzr, [sp], #16
        ldp x28, x29, [sp], #16
        ldp x26, x27, [sp], #16
        ldp x24, x25, [sp], #16
        ldp x22, x23, [sp], #16
        ldp x20, x21, [sp], #16
        ldp x18, x19, [sp], #16
        ldp x16, x17, [sp], #16
        ldp x14, x15, [sp], #16
        ldp x12, x13, [sp], #16
        ldp x10, x11, [sp], #16
        ldp x8, x9, [sp], #16
        ldp x6, x7, [sp], #16
        ldp x4, x5, [sp], #16
        ldp x2, x3, [sp], #16
        ldp x0, x1, [sp], #16
        eret
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
