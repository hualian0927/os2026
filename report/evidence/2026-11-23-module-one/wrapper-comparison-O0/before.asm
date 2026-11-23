
/home/lhl/os2026/lab5/evidence/2026-11-23-module-one/wrapper-comparison-O0/before.o:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <syscall>:
#include <syscall.h>

#define MAX_ARGS            5

static inline int
syscall(int64_t num, ...) {
   0:	7135                	addi	sp,sp,-160
   2:	eca2                	sd	s0,88(sp)
   4:	1080                	addi	s0,sp,96
   6:	faa43423          	sd	a0,-88(s0)
   a:	e40c                	sd	a1,8(s0)
   c:	e810                	sd	a2,16(s0)
   e:	ec14                	sd	a3,24(s0)
  10:	f018                	sd	a4,32(s0)
  12:	f41c                	sd	a5,40(s0)
  14:	03043823          	sd	a6,48(s0)
  18:	03143c23          	sd	a7,56(s0)
    va_list ap;
    va_start(ap, num);
  1c:	04040793          	addi	a5,s0,64
  20:	faf43023          	sd	a5,-96(s0)
  24:	fa043783          	ld	a5,-96(s0)
  28:	fc878793          	addi	a5,a5,-56
  2c:	fef43023          	sd	a5,-32(s0)
    uint64_t a[MAX_ARGS];
    int i, ret;
    for (i = 0; i < MAX_ARGS; i ++) {
  30:	fe042623          	sw	zero,-20(s0)
  34:	a025                	j	5c <.L2>
			34: R_RISCV_RVC_JUMP	.L2

0000000000000036 <.L3>:
        a[i] = va_arg(ap, uint64_t);
  36:	fe043783          	ld	a5,-32(s0)
  3a:	00878713          	addi	a4,a5,8
  3e:	fee43023          	sd	a4,-32(s0)
  42:	6398                	ld	a4,0(a5)
  44:	fec42783          	lw	a5,-20(s0)
  48:	078e                	slli	a5,a5,0x3
  4a:	17c1                	addi	a5,a5,-16
  4c:	97a2                	add	a5,a5,s0
  4e:	fce7b423          	sd	a4,-56(a5)
    for (i = 0; i < MAX_ARGS; i ++) {
  52:	fec42783          	lw	a5,-20(s0)
  56:	2785                	addiw	a5,a5,1
  58:	fef42623          	sw	a5,-20(s0)

000000000000005c <.L2>:
  5c:	fec42783          	lw	a5,-20(s0)
  60:	0007871b          	sext.w	a4,a5
  64:	4791                	li	a5,4
  66:	fce7d8e3          	bge	a5,a4,36 <.L3>
			66: R_RISCV_BRANCH	.L3
    }
    va_end(ap);

    asm volatile (
  6a:	fa843503          	ld	a0,-88(s0)
  6e:	fb843583          	ld	a1,-72(s0)
  72:	fc043603          	ld	a2,-64(s0)
  76:	fc843683          	ld	a3,-56(s0)
  7a:	fd043703          	ld	a4,-48(s0)
  7e:	fd843783          	ld	a5,-40(s0)
  82:	00000073          	ecall
  86:	faa43a23          	sd	a0,-76(s0)
        "ecall\n"
        "sd a0, %0"
        : "=m" (ret)
        : "m"(num), "m"(a[0]), "m"(a[1]), "m"(a[2]), "m"(a[3]), "m"(a[4])
        :"memory");
    return ret;
  8a:	fb442783          	lw	a5,-76(s0)
}
  8e:	853e                	mv	a0,a5
  90:	6466                	ld	s0,88(sp)
  92:	610d                	addi	sp,sp,160
  94:	8082                	ret

0000000000000096 <sys_exit>:

int
sys_exit(int64_t error_code) {
  96:	1101                	addi	sp,sp,-32
  98:	ec06                	sd	ra,24(sp)
  9a:	e822                	sd	s0,16(sp)
  9c:	1000                	addi	s0,sp,32
  9e:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_exit, error_code);
  a2:	fe843583          	ld	a1,-24(s0)
  a6:	4505                	li	a0,1
  a8:	00000097          	auipc	ra,0x0
			a8: R_RISCV_CALL_PLT	syscall
			a8: R_RISCV_RELAX	*ABS*
  ac:	000080e7          	jalr	ra # a8 <sys_exit+0x12>
  b0:	87aa                	mv	a5,a0
}
  b2:	853e                	mv	a0,a5
  b4:	60e2                	ld	ra,24(sp)
  b6:	6442                	ld	s0,16(sp)
  b8:	6105                	addi	sp,sp,32
  ba:	8082                	ret

00000000000000bc <sys_fork>:

int
sys_fork(void) {
  bc:	1141                	addi	sp,sp,-16
  be:	e406                	sd	ra,8(sp)
  c0:	e022                	sd	s0,0(sp)
  c2:	0800                	addi	s0,sp,16
    return syscall(SYS_fork);
  c4:	4509                	li	a0,2
  c6:	00000097          	auipc	ra,0x0
			c6: R_RISCV_CALL_PLT	syscall
			c6: R_RISCV_RELAX	*ABS*
  ca:	000080e7          	jalr	ra # c6 <sys_fork+0xa>
  ce:	87aa                	mv	a5,a0
}
  d0:	853e                	mv	a0,a5
  d2:	60a2                	ld	ra,8(sp)
  d4:	6402                	ld	s0,0(sp)
  d6:	0141                	addi	sp,sp,16
  d8:	8082                	ret

00000000000000da <sys_wait>:

int
sys_wait(int64_t pid, int *store) {
  da:	1101                	addi	sp,sp,-32
  dc:	ec06                	sd	ra,24(sp)
  de:	e822                	sd	s0,16(sp)
  e0:	1000                	addi	s0,sp,32
  e2:	fea43423          	sd	a0,-24(s0)
  e6:	feb43023          	sd	a1,-32(s0)
    return syscall(SYS_wait, pid, store);
  ea:	fe043603          	ld	a2,-32(s0)
  ee:	fe843583          	ld	a1,-24(s0)
  f2:	450d                	li	a0,3
  f4:	00000097          	auipc	ra,0x0
			f4: R_RISCV_CALL_PLT	syscall
			f4: R_RISCV_RELAX	*ABS*
  f8:	000080e7          	jalr	ra # f4 <sys_wait+0x1a>
  fc:	87aa                	mv	a5,a0
}
  fe:	853e                	mv	a0,a5
 100:	60e2                	ld	ra,24(sp)
 102:	6442                	ld	s0,16(sp)
 104:	6105                	addi	sp,sp,32
 106:	8082                	ret

0000000000000108 <sys_yield>:

int
sys_yield(void) {
 108:	1141                	addi	sp,sp,-16
 10a:	e406                	sd	ra,8(sp)
 10c:	e022                	sd	s0,0(sp)
 10e:	0800                	addi	s0,sp,16
    return syscall(SYS_yield);
 110:	4529                	li	a0,10
 112:	00000097          	auipc	ra,0x0
			112: R_RISCV_CALL_PLT	syscall
			112: R_RISCV_RELAX	*ABS*
 116:	000080e7          	jalr	ra # 112 <sys_yield+0xa>
 11a:	87aa                	mv	a5,a0
}
 11c:	853e                	mv	a0,a5
 11e:	60a2                	ld	ra,8(sp)
 120:	6402                	ld	s0,0(sp)
 122:	0141                	addi	sp,sp,16
 124:	8082                	ret

0000000000000126 <sys_kill>:

int
sys_kill(int64_t pid) {
 126:	1101                	addi	sp,sp,-32
 128:	ec06                	sd	ra,24(sp)
 12a:	e822                	sd	s0,16(sp)
 12c:	1000                	addi	s0,sp,32
 12e:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_kill, pid);
 132:	fe843583          	ld	a1,-24(s0)
 136:	4531                	li	a0,12
 138:	00000097          	auipc	ra,0x0
			138: R_RISCV_CALL_PLT	syscall
			138: R_RISCV_RELAX	*ABS*
 13c:	000080e7          	jalr	ra # 138 <sys_kill+0x12>
 140:	87aa                	mv	a5,a0
}
 142:	853e                	mv	a0,a5
 144:	60e2                	ld	ra,24(sp)
 146:	6442                	ld	s0,16(sp)
 148:	6105                	addi	sp,sp,32
 14a:	8082                	ret

000000000000014c <sys_getpid>:

int
sys_getpid(void) {
 14c:	1141                	addi	sp,sp,-16
 14e:	e406                	sd	ra,8(sp)
 150:	e022                	sd	s0,0(sp)
 152:	0800                	addi	s0,sp,16
    return syscall(SYS_getpid);
 154:	4549                	li	a0,18
 156:	00000097          	auipc	ra,0x0
			156: R_RISCV_CALL_PLT	syscall
			156: R_RISCV_RELAX	*ABS*
 15a:	000080e7          	jalr	ra # 156 <sys_getpid+0xa>
 15e:	87aa                	mv	a5,a0
}
 160:	853e                	mv	a0,a5
 162:	60a2                	ld	ra,8(sp)
 164:	6402                	ld	s0,0(sp)
 166:	0141                	addi	sp,sp,16
 168:	8082                	ret

000000000000016a <sys_putc>:

int
sys_putc(int64_t c) {
 16a:	1101                	addi	sp,sp,-32
 16c:	ec06                	sd	ra,24(sp)
 16e:	e822                	sd	s0,16(sp)
 170:	1000                	addi	s0,sp,32
 172:	fea43423          	sd	a0,-24(s0)
    return syscall(SYS_putc, c);
 176:	fe843583          	ld	a1,-24(s0)
 17a:	4579                	li	a0,30
 17c:	00000097          	auipc	ra,0x0
			17c: R_RISCV_CALL_PLT	syscall
			17c: R_RISCV_RELAX	*ABS*
 180:	000080e7          	jalr	ra # 17c <sys_putc+0x12>
 184:	87aa                	mv	a5,a0
}
 186:	853e                	mv	a0,a5
 188:	60e2                	ld	ra,24(sp)
 18a:	6442                	ld	s0,16(sp)
 18c:	6105                	addi	sp,sp,32
 18e:	8082                	ret

0000000000000190 <sys_pgdir>:

int
sys_pgdir(void) {
 190:	1141                	addi	sp,sp,-16
 192:	e406                	sd	ra,8(sp)
 194:	e022                	sd	s0,0(sp)
 196:	0800                	addi	s0,sp,16
    return syscall(SYS_pgdir);
 198:	457d                	li	a0,31
 19a:	00000097          	auipc	ra,0x0
			19a: R_RISCV_CALL_PLT	syscall
			19a: R_RISCV_RELAX	*ABS*
 19e:	000080e7          	jalr	ra # 19a <sys_pgdir+0xa>
 1a2:	87aa                	mv	a5,a0
}
 1a4:	853e                	mv	a0,a5
 1a6:	60a2                	ld	ra,8(sp)
 1a8:	6402                	ld	s0,0(sp)
 1aa:	0141                	addi	sp,sp,16
 1ac:	8082                	ret
