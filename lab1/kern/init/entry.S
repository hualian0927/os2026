#include <mmu.h>
#include <memlayout.h>

    .section .text.kern_entry,"ax",%progbits
    .globl kern_entry
kern_entry:
    # The stack storage is reserved below; initialize sp before entering C.
    la sp, bootstacktop

    # kern_init never returns: tail does not create a new return address.
    tail kern_init

.section .data
    # .align 2^12
    .align PGSHIFT
    .global bootstack
bootstack:
    .space KSTACKSIZE
    .global bootstacktop
bootstacktop:
