/* Ordinary ABI-preserving test calls; Trap save/restore comes ONLY from trapentry.S. */
#include <riscv.h>
.text
.option push
.option norelax
.option norvc

/* 128-byte ordinary call frame: RA and s0-s11, plus snapshot scratch. */
.macro ABI_ENTER
    addi sp, sp, -128
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    sd s6, 56(sp)
    sd s7, 64(sp)
    sd s8, 72(sp)
    sd s9, 80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)
.endm
.macro ABI_LEAVE
    ld ra, 0(sp)
    ld s0, 8(sp)
    ld s1, 16(sp)
    ld s2, 24(sp)
    ld s3, 32(sp)
    ld s4, 40(sp)
    ld s5, 48(sp)
    ld s6, 56(sp)
    ld s7, 64(sp)
    ld s8, 72(sp)
    ld s9, 80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)
    addi sp, sp, 128
.endm

.balign 4
.globl abi_checked_probe
abi_checked_probe:
    ABI_ENTER
    li s0, 0x508
    li s1, 0x509
    li s2, 0x512
    li s3, 0x513
    li s4, 0x514
    li s5, 0x515
    li s6, 0x516
    li s7, 0x517
    li s8, 0x518
    li s9, 0x519
    li s10, 0x51a
    li s11, 0x51b
    la t0, wrapper_sp_before
    sd sp, 0(t0)
    call module_probe
    la t0, wrapper_sp_after
    sd sp, 0(t0)
    li t0, 0x508
    bne s0, t0, .Labi_bad
    li t0, 0x509
    bne s1, t0, .Labi_bad
    li t0, 0x512
    bne s2, t0, .Labi_bad
    li t0, 0x513
    bne s3, t0, .Labi_bad
    li t0, 0x514
    bne s4, t0, .Labi_bad
    li t0, 0x515
    bne s5, t0, .Labi_bad
    li t0, 0x516
    bne s6, t0, .Labi_bad
    li t0, 0x517
    bne s7, t0, .Labi_bad
    li t0, 0x518
    bne s8, t0, .Labi_bad
    li t0, 0x519
    bne s9, t0, .Labi_bad
    li t0, 0x51a
    bne s10, t0, .Labi_bad
    li t0, 0x51b
    bne s11, t0, .Labi_bad
    li a0, 0
    j .Labi_done
.Labi_bad:
    li a0, 1
.Labi_done:
    ABI_LEAVE
    ret

.balign 4
.globl module_probe
module_probe:
    ABI_ENTER
    la t0, expected_sp
    sd sp, 0(t0)
    la t0, expected_gp
    sd gp, 0(t0)
    la t0, expected_tp
    sd tp, 0(t0)
    li x1, 0x101
    li x5, 0x105
    li x6, 0x106
    li x7, 0x107
    li x8, 0x108
    li x9, 0x109
    li x10, 0x10a
    li x11, 0x10b
    li x12, 0x10c
    li x13, 0x10d
    li x14, 0x10e
    li x15, 0x10f
    li x16, 0x110
    li x17, 0x111
    li x18, 0x112
    li x19, 0x113
    li x20, 0x114
    li x21, 0x115
    li x22, 0x116
    li x23, 0x117
    li x24, 0x118
    li x25, 0x119
    li x26, 0x11a
    li x27, 0x11b
    li x28, 0x11c
    li x29, 0x11d
    li x30, 0x11e
    li x31, 0x11f
.globl probe_breakpoint
probe_breakpoint:
    .4byte 0x00100073
    .4byte 0x00000000
.globl probe_resume
probe_resume:
    /* Preserve t0 before using it as the snapshot base. Do not repair Trap results. */
    sd t0, 104(sp)
    la t0, observed
    sd x0, 0*8(t0)
    sd x1, 1*8(t0)
    sd x2, 2*8(t0)
    sd x3, 3*8(t0)
    sd x4, 4*8(t0)
    sd x6, 6*8(t0)
    sd x7, 7*8(t0)
    sd x8, 8*8(t0)
    sd x9, 9*8(t0)
    sd x10, 10*8(t0)
    sd x11, 11*8(t0)
    sd x12, 12*8(t0)
    sd x13, 13*8(t0)
    sd x14, 14*8(t0)
    sd x15, 15*8(t0)
    sd x16, 16*8(t0)
    sd x17, 17*8(t0)
    sd x18, 18*8(t0)
    sd x19, 19*8(t0)
    sd x20, 20*8(t0)
    sd x21, 21*8(t0)
    sd x22, 22*8(t0)
    sd x23, 23*8(t0)
    sd x24, 24*8(t0)
    sd x25, 25*8(t0)
    sd x26, 26*8(t0)
    sd x27, 27*8(t0)
    sd x28, 28*8(t0)
    sd x29, 29*8(t0)
    sd x30, 30*8(t0)
    sd x31, 31*8(t0)
    ld t1, 104(sp)
    sd t1, 5*8(t0)
    ABI_LEAVE
    ret
.option pop
