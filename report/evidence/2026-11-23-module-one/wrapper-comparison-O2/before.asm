
/home/lhl/os2026/lab5/evidence/2026-11-23-module-one/wrapper-comparison-O2/before.o:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <syscall>:
#include <syscall.h>

#define MAX_ARGS            5

static inline int
syscall(int64_t num, ...) {
   0:	7175                	addi	sp,sp,-144
   2:	e42a                	sd	a0,8(sp)
    va_list ap;
    va_start(ap, num);
    uint64_t a[MAX_ARGS];
    int i, ret;
    for (i = 0; i < MAX_ARGS; i ++) {
        a[i] = va_arg(ap, uint64_t);
   4:	0108                	addi	a0,sp,128

0000000000000006 <.LVL1>:
syscall(int64_t num, ...) {
   6:	ecae                	sd	a1,88(sp)
   8:	f0b2                	sd	a2,96(sp)
   a:	f4b6                	sd	a3,104(sp)
   c:	f8ba                	sd	a4,112(sp)
   e:	fcbe                	sd	a5,120(sp)
  10:	e142                	sd	a6,128(sp)
  12:	e546                	sd	a7,136(sp)
        a[i] = va_arg(ap, uint64_t);
  14:	f02a                	sd	a0,32(sp)
  16:	f42e                	sd	a1,40(sp)
  18:	f832                	sd	a2,48(sp)
  1a:	fc36                	sd	a3,56(sp)
  1c:	e0ba                	sd	a4,64(sp)
  1e:	e4be                	sd	a5,72(sp)

0000000000000020 <.LVL2>:
    }
    va_end(ap);

    asm volatile (
  20:	6522                	ld	a0,8(sp)
  22:	75a2                	ld	a1,40(sp)
  24:	7642                	ld	a2,48(sp)
  26:	76e2                	ld	a3,56(sp)
  28:	6706                	ld	a4,64(sp)
  2a:	67a6                	ld	a5,72(sp)
  2c:	00000073          	ecall
  30:	00a13e23          	sd	a0,28(sp)
        "sd a0, %0"
        : "=m" (ret)
        : "m"(num), "m"(a[0]), "m"(a[1]), "m"(a[2]), "m"(a[3]), "m"(a[4])
        :"memory");
    return ret;
}
  34:	4572                	lw	a0,28(sp)
  36:	6149                	addi	sp,sp,144

0000000000000038 <.LVL3>:
  38:	8082                	ret

000000000000003a <sys_exit>:

int
sys_exit(int64_t error_code) {
  3a:	85aa                	mv	a1,a0
    return syscall(SYS_exit, error_code);
  3c:	4505                	li	a0,1

000000000000003e <.LVL5>:
  3e:	00000317          	auipc	t1,0x0
			3e: R_RISCV_CALL_PLT	syscall
			3e: R_RISCV_RELAX	*ABS*
  42:	00030067          	jr	t1 # 3e <.LVL5>

0000000000000046 <sys_fork>:
}

int
sys_fork(void) {
    return syscall(SYS_fork);
  46:	4509                	li	a0,2
  48:	00000317          	auipc	t1,0x0
			48: R_RISCV_CALL_PLT	syscall
			48: R_RISCV_RELAX	*ABS*
  4c:	00030067          	jr	t1 # 48 <sys_fork+0x2>

0000000000000050 <sys_wait>:
}

int
sys_wait(int64_t pid, int *store) {
  50:	862e                	mv	a2,a1
    return syscall(SYS_wait, pid, store);
  52:	85aa                	mv	a1,a0

0000000000000054 <.LVL9>:
  54:	450d                	li	a0,3

0000000000000056 <.LVL10>:
  56:	00000317          	auipc	t1,0x0
			56: R_RISCV_CALL_PLT	syscall
			56: R_RISCV_RELAX	*ABS*
  5a:	00030067          	jr	t1 # 56 <.LVL10>

000000000000005e <sys_yield>:
}

int
sys_yield(void) {
    return syscall(SYS_yield);
  5e:	4529                	li	a0,10
  60:	00000317          	auipc	t1,0x0
			60: R_RISCV_CALL_PLT	syscall
			60: R_RISCV_RELAX	*ABS*
  64:	00030067          	jr	t1 # 60 <sys_yield+0x2>

0000000000000068 <sys_kill>:
}

int
sys_kill(int64_t pid) {
  68:	85aa                	mv	a1,a0
    return syscall(SYS_kill, pid);
  6a:	4531                	li	a0,12

000000000000006c <.LVL14>:
  6c:	00000317          	auipc	t1,0x0
			6c: R_RISCV_CALL_PLT	syscall
			6c: R_RISCV_RELAX	*ABS*
  70:	00030067          	jr	t1 # 6c <.LVL14>

0000000000000074 <sys_getpid>:
}

int
sys_getpid(void) {
    return syscall(SYS_getpid);
  74:	4549                	li	a0,18
  76:	00000317          	auipc	t1,0x0
			76: R_RISCV_CALL_PLT	syscall
			76: R_RISCV_RELAX	*ABS*
  7a:	00030067          	jr	t1 # 76 <sys_getpid+0x2>

000000000000007e <sys_putc>:
}

int
sys_putc(int64_t c) {
  7e:	85aa                	mv	a1,a0
    return syscall(SYS_putc, c);
  80:	4579                	li	a0,30

0000000000000082 <.LVL18>:
  82:	00000317          	auipc	t1,0x0
			82: R_RISCV_CALL_PLT	syscall
			82: R_RISCV_RELAX	*ABS*
  86:	00030067          	jr	t1 # 82 <.LVL18>

000000000000008a <sys_pgdir>:
}

int
sys_pgdir(void) {
    return syscall(SYS_pgdir);
  8a:	457d                	li	a0,31
  8c:	00000317          	auipc	t1,0x0
			8c: R_RISCV_CALL_PLT	syscall
			8c: R_RISCV_RELAX	*ABS*
  90:	00030067          	jr	t1 # 8c <sys_pgdir+0x2>
