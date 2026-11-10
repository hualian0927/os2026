
/home/lhl/os2026/lab4/report/evidence/2026-11-10-page-mapping/checks-birpehib/software-O2/kernel:     file format elf64-littleriscv


Disassembly of section .text:

ffffffffc02011e0 <page_remove>:
ffffffffc02011e0:	1141                	addi	sp,sp,-16
ffffffffc02011e2:	4601                	li	a2,0
ffffffffc02011e4:	e022                	sd	s0,0(sp)
ffffffffc02011e6:	e406                	sd	ra,8(sp)
ffffffffc02011e8:	842e                	mv	s0,a1
ffffffffc02011ea:	b95ff0ef          	jal	ffffffffc0200d7e <get_pte>
ffffffffc02011ee:	c519                	beqz	a0,ffffffffc02011fc <page_remove+0x1c>
ffffffffc02011f0:	85aa                	mv	a1,a0
ffffffffc02011f2:	8522                	mv	a0,s0
ffffffffc02011f4:	6402                	ld	s0,0(sp)
ffffffffc02011f6:	60a2                	ld	ra,8(sp)
ffffffffc02011f8:	0141                	addi	sp,sp,16
ffffffffc02011fa:	b579                	j	ffffffffc0201088 <page_remove_pte.isra.0>
ffffffffc02011fc:	60a2                	ld	ra,8(sp)
ffffffffc02011fe:	6402                	ld	s0,0(sp)
ffffffffc0201200:	0141                	addi	sp,sp,16
ffffffffc0201202:	8082                	ret
