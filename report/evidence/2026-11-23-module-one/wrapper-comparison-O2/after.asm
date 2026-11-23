
/home/lhl/os2026/lab5/evidence/2026-11-23-module-one/wrapper-comparison-O2/after.o:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <sys_exit>:
        : "memory");
    return (int)(int64_t)r0;
}

int
sys_exit(int64_t error_code) {
   0:	85aa                	mv	a1,a0

0000000000000002 <.LBB18>:
    register uint64_t r2 asm("a2") = arg1;
   2:	4601                	li	a2,0
    register uint64_t r0 asm("a0") = (uint64_t)num;
   4:	4505                	li	a0,1

0000000000000006 <.LVL2>:
    register uint64_t r3 asm("a3") = arg2;
   6:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
   8:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
   a:	4781                	li	a5,0
    asm volatile ("ecall"
   c:	00000073          	ecall

0000000000000010 <.LBE18>:
    return syscall(SYS_exit, (uint64_t)error_code, 0, 0, 0, 0);
}
  10:	2501                	sext.w	a0,a0
  12:	8082                	ret

0000000000000014 <sys_fork>:
    register uint64_t r0 asm("a0") = (uint64_t)num;
  14:	4509                	li	a0,2
    register uint64_t r1 asm("a1") = arg0;
  16:	4581                	li	a1,0
    register uint64_t r2 asm("a2") = arg1;
  18:	4601                	li	a2,0
    register uint64_t r3 asm("a3") = arg2;
  1a:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  1c:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  1e:	4781                	li	a5,0
    asm volatile ("ecall"
  20:	00000073          	ecall

0000000000000024 <.LBE20>:

int
sys_fork(void) {
    return syscall(SYS_fork, 0, 0, 0, 0, 0);
}
  24:	2501                	sext.w	a0,a0
  26:	8082                	ret

0000000000000028 <sys_wait>:

int
sys_wait(int64_t pid, int *store) {
  28:	87aa                	mv	a5,a0

000000000000002a <.LVL7>:
  2a:	862e                	mv	a2,a1

000000000000002c <.LBB22>:
    register uint64_t r0 asm("a0") = (uint64_t)num;
  2c:	450d                	li	a0,3

000000000000002e <.LVL9>:
    register uint64_t r1 asm("a1") = arg0;
  2e:	85be                	mv	a1,a5

0000000000000030 <.LVL10>:
    register uint64_t r3 asm("a3") = arg2;
  30:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  32:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  34:	4781                	li	a5,0

0000000000000036 <.LVL11>:
    asm volatile ("ecall"
  36:	00000073          	ecall

000000000000003a <.LBE22>:
    return syscall(SYS_wait, (uint64_t)pid, (uintptr_t)store, 0, 0, 0);
}
  3a:	2501                	sext.w	a0,a0
  3c:	8082                	ret

000000000000003e <sys_yield>:
    register uint64_t r0 asm("a0") = (uint64_t)num;
  3e:	4529                	li	a0,10
    register uint64_t r1 asm("a1") = arg0;
  40:	4581                	li	a1,0
    register uint64_t r2 asm("a2") = arg1;
  42:	4601                	li	a2,0
    register uint64_t r3 asm("a3") = arg2;
  44:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  46:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  48:	4781                	li	a5,0
    asm volatile ("ecall"
  4a:	00000073          	ecall

000000000000004e <.LBE24>:

int
sys_yield(void) {
    return syscall(SYS_yield, 0, 0, 0, 0, 0);
}
  4e:	2501                	sext.w	a0,a0
  50:	8082                	ret

0000000000000052 <sys_kill>:

int
sys_kill(int64_t pid) {
  52:	85aa                	mv	a1,a0

0000000000000054 <.LBB26>:
    register uint64_t r2 asm("a2") = arg1;
  54:	4601                	li	a2,0
    register uint64_t r0 asm("a0") = (uint64_t)num;
  56:	4531                	li	a0,12

0000000000000058 <.LVL17>:
    register uint64_t r3 asm("a3") = arg2;
  58:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  5a:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  5c:	4781                	li	a5,0
    asm volatile ("ecall"
  5e:	00000073          	ecall

0000000000000062 <.LBE26>:
    return syscall(SYS_kill, (uint64_t)pid, 0, 0, 0, 0);
}
  62:	2501                	sext.w	a0,a0
  64:	8082                	ret

0000000000000066 <sys_getpid>:
    register uint64_t r0 asm("a0") = (uint64_t)num;
  66:	4549                	li	a0,18
    register uint64_t r1 asm("a1") = arg0;
  68:	4581                	li	a1,0
    register uint64_t r2 asm("a2") = arg1;
  6a:	4601                	li	a2,0
    register uint64_t r3 asm("a3") = arg2;
  6c:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  6e:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  70:	4781                	li	a5,0
    asm volatile ("ecall"
  72:	00000073          	ecall

0000000000000076 <.LBE28>:

int
sys_getpid(void) {
    return syscall(SYS_getpid, 0, 0, 0, 0, 0);
}
  76:	2501                	sext.w	a0,a0
  78:	8082                	ret

000000000000007a <sys_putc>:

int
sys_putc(int64_t c) {
  7a:	85aa                	mv	a1,a0

000000000000007c <.LBB30>:
    register uint64_t r2 asm("a2") = arg1;
  7c:	4601                	li	a2,0
    register uint64_t r0 asm("a0") = (uint64_t)num;
  7e:	4579                	li	a0,30

0000000000000080 <.LVL23>:
    register uint64_t r3 asm("a3") = arg2;
  80:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  82:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  84:	4781                	li	a5,0
    asm volatile ("ecall"
  86:	00000073          	ecall

000000000000008a <.LBE30>:
    return syscall(SYS_putc, (uint64_t)c, 0, 0, 0, 0);
}
  8a:	2501                	sext.w	a0,a0
  8c:	8082                	ret

000000000000008e <sys_pgdir>:
    register uint64_t r0 asm("a0") = (uint64_t)num;
  8e:	457d                	li	a0,31
    register uint64_t r1 asm("a1") = arg0;
  90:	4581                	li	a1,0
    register uint64_t r2 asm("a2") = arg1;
  92:	4601                	li	a2,0
    register uint64_t r3 asm("a3") = arg2;
  94:	4681                	li	a3,0
    register uint64_t r4 asm("a4") = arg3;
  96:	4701                	li	a4,0
    register uint64_t r5 asm("a5") = arg4;
  98:	4781                	li	a5,0
    asm volatile ("ecall"
  9a:	00000073          	ecall

000000000000009e <.LBE32>:

int
sys_pgdir(void) {
    return syscall(SYS_pgdir, 0, 0, 0, 0, 0);
}
  9e:	2501                	sext.w	a0,a0
  a0:	8082                	ret
