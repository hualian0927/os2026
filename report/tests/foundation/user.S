#include <riscv.h>
.text
.option push
.option norelax
.option norvc
.globl enter_user_fixture
enter_user_fixture:
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
    sd gp, 104(sp)
    sd tp, 112(sp)
    la t0, kernel_resume_sp
    sd sp, 0(t0)
    # Uses ACTUAL production forkrets -> RESTORE_ALL -> sret.
    call forkrets
.globl user_fixture_return
user_fixture_return:
    csrr t0, sstatus
    la t1, final_status
    sd t0, 0(t1)
    csrr t0, sscratch
    la t1, final_sscratch
    sd t0, 0(t1)
    csrci sstatus, 2
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
    ld gp, 104(sp)
    ld tp, 112(sp)
    addi sp, sp, 128
    ret

# A position-independent controlled U-mode probe, not an ELF loader or syscall implementation.
.section .rodata
.balign 4
.globl user_probe_begin,user_probe_end
user_probe_begin:
.Lagain:
    li a0, 0x5a5
    li t0, 0x123
    li s0, 0x456
    addi sp, sp, -16
    sd a0, 0(sp)
    ld a1, 0(sp)
    ecall
    addi sp, sp, 16
    li t1, 0xabc
    bne a0, t1, .Lbad
    li t1, 0xdef
    bne t0, t1, .Lbad
    li t1, 0x789
    bne s0, t1, .Lbad
    li t1, 0x5a5
    bne a1, t1, .Lbad
    addi s1, s1, -1
    bnez s1, .Lagain
    li a0, 0xcafe
    ecall
.Lbad:
    li a0, 0xbad
    ecall
    j .Lbad
user_probe_end:
.option pop
