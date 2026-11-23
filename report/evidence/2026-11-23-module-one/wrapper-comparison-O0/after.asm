
/home/lhl/os2026/lab5/evidence/2026-11-23-module-one/wrapper-comparison-O0/after.o:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <syscall>:
#include <syscall.h>

//lab5 function1 2412351: retain the public ABI; supply all five argument slots.
static inline int
syscall(int64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2,
        uint64_t arg3, uint64_t arg4) {
   0:	7139                	addi	sp,sp,-64
   2:	fc22                	sd	s0,56(sp)
   4:	0080                	addi	s0,sp,64
   6:	fea43423          	sd	a0,-24(s0)
   a:	feb43023          	sd	a1,-32(s0)
   e:	fcc43c23          	sd	a2,-40(s0)
  12:	fcd43823          	sd	a3,-48(s0)
  16:	fce43423          	sd	a4,-56(s0)
  1a:	fcf43023          	sd	a5,-64(s0)
    register uint64_t r0 asm("a0") = (uint64_t)num;
  1e:	fe843783          	ld	a5,-24(s0)
  22:	853e                	mv	a0,a5
    register uint64_t r1 asm("a1") = arg0;
  24:	fe043583          	ld	a1,-32(s0)
    register uint64_t r2 asm("a2") = arg1;
  28:	fd843603          	ld	a2,-40(s0)
    register uint64_t r3 asm("a3") = arg2;
  2c:	fd043683          	ld	a3,-48(s0)
    register uint64_t r4 asm("a4") = arg3;
  30:	fc843703          	ld	a4,-56(s0)
    register uint64_t r5 asm("a5") = arg4;
  34:	fc043783          	ld	a5,-64(s0)
    asm volatile ("ecall"
  38:	00000073          	ecall
        : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4), "+r"(r5)
        :
        : "memory");
    return (int)(int64_t)r0;
  3c:	87aa                	mv	a5,a0
  3e:	2781                	sext.w	a5,a5
}
  40:	853e                	mv	a0,a5
  42:	7462                	ld	s0,56(sp)
  44:	6121                	addi	sp,sp,64
  46:	8082                	ret

0000000000000048 <sys_exit>:

int
sys_exit(int64_t error_code) {
  48:	1101                	addi	sp,sp,-32
  4a:	ec06                	sd	ra,24(sp)
  4c:	e822                	sd	s0,16(sp)
  4e:	1000                	addi	s0,sp,32
  50:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_exit, (uint64_t)error_code, 0, 0, 0, 0);
  54:	fe843583          	ld	a1,-24(s0)
  58:	4781                	li	a5,0
  5a:	4701                	li	a4,0
  5c:	4681                	li	a3,0
  5e:	4601                	li	a2,0
  60:	4505                	li	a0,1
  62:	00000097          	auipc	ra,0x0
			62: R_RISCV_CALL_PLT	syscall
			62: R_RISCV_RELAX	*ABS*
  66:	000080e7          	jalr	ra # 62 <sys_exit+0x1a>
  6a:	87aa                	mv	a5,a0
}
  6c:	853e                	mv	a0,a5
  6e:	60e2                	ld	ra,24(sp)
  70:	6442                	ld	s0,16(sp)
  72:	6105                	addi	sp,sp,32
  74:	8082                	ret

0000000000000076 <sys_fork>:

int
sys_fork(void) {
  76:	1141                	addi	sp,sp,-16
  78:	e406                	sd	ra,8(sp)
  7a:	e022                	sd	s0,0(sp)
  7c:	0800                	addi	s0,sp,16
    return syscall(SYS_fork, 0, 0, 0, 0, 0);
  7e:	4781                	li	a5,0
  80:	4701                	li	a4,0
  82:	4681                	li	a3,0
  84:	4601                	li	a2,0
  86:	4581                	li	a1,0
  88:	4509                	li	a0,2
  8a:	00000097          	auipc	ra,0x0
			8a: R_RISCV_CALL_PLT	syscall
			8a: R_RISCV_RELAX	*ABS*
  8e:	000080e7          	jalr	ra # 8a <sys_fork+0x14>
  92:	87aa                	mv	a5,a0
}
  94:	853e                	mv	a0,a5
  96:	60a2                	ld	ra,8(sp)
  98:	6402                	ld	s0,0(sp)
  9a:	0141                	addi	sp,sp,16
  9c:	8082                	ret

000000000000009e <sys_wait>:

int
sys_wait(int64_t pid, int *store) {
  9e:	1101                	addi	sp,sp,-32
  a0:	ec06                	sd	ra,24(sp)
  a2:	e822                	sd	s0,16(sp)
  a4:	1000                	addi	s0,sp,32
  a6:	fea43423          	sd	a0,-24(s0)
  aa:	feb43023          	sd	a1,-32(s0)
    return syscall(SYS_wait, (uint64_t)pid, (uintptr_t)store, 0, 0, 0);
  ae:	fe843583          	ld	a1,-24(s0)
  b2:	fe043603          	ld	a2,-32(s0)
  b6:	4781                	li	a5,0
  b8:	4701                	li	a4,0
  ba:	4681                	li	a3,0
  bc:	450d                	li	a0,3
  be:	00000097          	auipc	ra,0x0
			be: R_RISCV_CALL_PLT	syscall
			be: R_RISCV_RELAX	*ABS*
  c2:	000080e7          	jalr	ra # be <sys_wait+0x20>
  c6:	87aa                	mv	a5,a0
}
  c8:	853e                	mv	a0,a5
  ca:	60e2                	ld	ra,24(sp)
  cc:	6442                	ld	s0,16(sp)
  ce:	6105                	addi	sp,sp,32
  d0:	8082                	ret

00000000000000d2 <sys_yield>:

int
sys_yield(void) {
  d2:	1141                	addi	sp,sp,-16
  d4:	e406                	sd	ra,8(sp)
  d6:	e022                	sd	s0,0(sp)
  d8:	0800                	addi	s0,sp,16
    return syscall(SYS_yield, 0, 0, 0, 0, 0);
  da:	4781                	li	a5,0
  dc:	4701                	li	a4,0
  de:	4681                	li	a3,0
  e0:	4601                	li	a2,0
  e2:	4581                	li	a1,0
  e4:	4529                	li	a0,10
  e6:	00000097          	auipc	ra,0x0
			e6: R_RISCV_CALL_PLT	syscall
			e6: R_RISCV_RELAX	*ABS*
  ea:	000080e7          	jalr	ra # e6 <sys_yield+0x14>
  ee:	87aa                	mv	a5,a0
}
  f0:	853e                	mv	a0,a5
  f2:	60a2                	ld	ra,8(sp)
  f4:	6402                	ld	s0,0(sp)
  f6:	0141                	addi	sp,sp,16
  f8:	8082                	ret

00000000000000fa <sys_kill>:

int
sys_kill(int64_t pid) {
  fa:	1101                	addi	sp,sp,-32
  fc:	ec06                	sd	ra,24(sp)
  fe:	e822                	sd	s0,16(sp)
 100:	1000                	addi	s0,sp,32
 102:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_kill, (uint64_t)pid, 0, 0, 0, 0);
 106:	fe843583          	ld	a1,-24(s0)
 10a:	4781                	li	a5,0
 10c:	4701                	li	a4,0
 10e:	4681                	li	a3,0
 110:	4601                	li	a2,0
 112:	4531                	li	a0,12
 114:	00000097          	auipc	ra,0x0
			114: R_RISCV_CALL_PLT	syscall
			114: R_RISCV_RELAX	*ABS*
 118:	000080e7          	jalr	ra # 114 <sys_kill+0x1a>
 11c:	87aa                	mv	a5,a0
}
 11e:	853e                	mv	a0,a5
 120:	60e2                	ld	ra,24(sp)
 122:	6442                	ld	s0,16(sp)
 124:	6105                	addi	sp,sp,32
 126:	8082                	ret

0000000000000128 <sys_getpid>:

int
sys_getpid(void) {
 128:	1141                	addi	sp,sp,-16
 12a:	e406                	sd	ra,8(sp)
 12c:	e022                	sd	s0,0(sp)
 12e:	0800                	addi	s0,sp,16
    return syscall(SYS_getpid, 0, 0, 0, 0, 0);
 130:	4781                	li	a5,0
 132:	4701                	li	a4,0
 134:	4681                	li	a3,0
 136:	4601                	li	a2,0
 138:	4581                	li	a1,0
 13a:	4549                	li	a0,18
 13c:	00000097          	auipc	ra,0x0
			13c: R_RISCV_CALL_PLT	syscall
			13c: R_RISCV_RELAX	*ABS*
 140:	000080e7          	jalr	ra # 13c <sys_getpid+0x14>
 144:	87aa                	mv	a5,a0
}
 146:	853e                	mv	a0,a5
 148:	60a2                	ld	ra,8(sp)
 14a:	6402                	ld	s0,0(sp)
 14c:	0141                	addi	sp,sp,16
 14e:	8082                	ret

0000000000000150 <sys_putc>:

int
sys_putc(int64_t c) {
 150:	1101                	addi	sp,sp,-32
 152:	ec06                	sd	ra,24(sp)
 154:	e822                	sd	s0,16(sp)
 156:	1000                	addi	s0,sp,32
 158:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_putc, (uint64_t)c, 0, 0, 0, 0);
 15c:	fe843583          	ld	a1,-24(s0)
 160:	4781                	li	a5,0
 162:	4701                	li	a4,0
 164:	4681                	li	a3,0
 166:	4601                	li	a2,0
 168:	4579                	li	a0,30
 16a:	00000097          	auipc	ra,0x0
			16a: R_RISCV_CALL_PLT	syscall
			16a: R_RISCV_RELAX	*ABS*
 16e:	000080e7          	jalr	ra # 16a <sys_putc+0x1a>
 172:	87aa                	mv	a5,a0
}
 174:	853e                	mv	a0,a5
 176:	60e2                	ld	ra,24(sp)
 178:	6442                	ld	s0,16(sp)
 17a:	6105                	addi	sp,sp,32
 17c:	8082                	ret

000000000000017e <sys_pgdir>:

int
sys_pgdir(void) {
 17e:	1141                	addi	sp,sp,-16
 180:	e406                	sd	ra,8(sp)
 182:	e022                	sd	s0,0(sp)
 184:	0800                	addi	s0,sp,16
    return syscall(SYS_pgdir, 0, 0, 0, 0, 0);
 186:	4781                	li	a5,0
 188:	4701                	li	a4,0
 18a:	4681                	li	a3,0
 18c:	4601                	li	a2,0
 18e:	4581                	li	a1,0
 190:	457d                	li	a0,31
 192:	00000097          	auipc	ra,0x0
			192: R_RISCV_CALL_PLT	syscall
			192: R_RISCV_RELAX	*ABS*
 196:	000080e7          	jalr	ra # 192 <sys_pgdir+0x14>
 19a:	87aa                	mv	a5,a0
}
 19c:	853e                	mv	a0,a5
 19e:	60a2                	ld	ra,8(sp)
 1a0:	6402                	ld	s0,0(sp)
 1a2:	0141                	addi	sp,sp,16
 1a4:	8082                	ret
