
/home/lhl/os2026/lab4/report/evidence/2026-11-10-page-mapping/checks-birpehib/software-O2/kernel:     file format elf64-littleriscv


Disassembly of section .text:

ffffffffc0201088 <page_remove_pte.isra.0>:
ffffffffc0201088:	619c                	ld	a5,0(a1)
ffffffffc020108a:	0017f713          	andi	a4,a5,1
ffffffffc020108e:	e311                	bnez	a4,ffffffffc0201092 <page_remove_pte.isra.0+0xa>
ffffffffc0201090:	8082                	ret
ffffffffc0201092:	1141                	addi	sp,sp,-16
ffffffffc0201094:	078a                	slli	a5,a5,0x2
ffffffffc0201096:	e406                	sd	ra,8(sp)
ffffffffc0201098:	e022                	sd	s0,0(sp)
ffffffffc020109a:	83b1                	srli	a5,a5,0xc
ffffffffc020109c:	00007717          	auipc	a4,0x7
ffffffffc02010a0:	fc473703          	ld	a4,-60(a4) # ffffffffc0208060 <npage>
ffffffffc02010a4:	06e7f963          	bgeu	a5,a4,ffffffffc0201116 <page_remove_pte.isra.0+0x8e>
ffffffffc02010a8:	fff80737          	lui	a4,0xfff80
ffffffffc02010ac:	97ba                	add	a5,a5,a4
ffffffffc02010ae:	079a                	slli	a5,a5,0x6
ffffffffc02010b0:	00007417          	auipc	s0,0x7
ffffffffc02010b4:	fb843403          	ld	s0,-72(s0) # ffffffffc0208068 <pages>
ffffffffc02010b8:	943e                	add	s0,s0,a5
ffffffffc02010ba:	401c                	lw	a5,0(s0)
ffffffffc02010bc:	04f05f63          	blez	a5,ffffffffc020111a <page_remove_pte.isra.0+0x92>
ffffffffc02010c0:	0005b023          	sd	zero,0(a1)
ffffffffc02010c4:	12050073          	sfence.vma	a0
ffffffffc02010c8:	401c                	lw	a5,0(s0)
ffffffffc02010ca:	fff7871b          	addiw	a4,a5,-1
ffffffffc02010ce:	c018                	sw	a4,0(s0)
ffffffffc02010d0:	c709                	beqz	a4,ffffffffc02010da <page_remove_pte.isra.0+0x52>
ffffffffc02010d2:	60a2                	ld	ra,8(sp)
ffffffffc02010d4:	6402                	ld	s0,0(sp)
ffffffffc02010d6:	0141                	addi	sp,sp,16
ffffffffc02010d8:	8082                	ret
ffffffffc02010da:	100027f3          	csrr	a5,sstatus
ffffffffc02010de:	8b89                	andi	a5,a5,2
ffffffffc02010e0:	ef81                	bnez	a5,ffffffffc02010f8 <page_remove_pte.isra.0+0x70>
ffffffffc02010e2:	00007797          	auipc	a5,0x7
ffffffffc02010e6:	f5e7b783          	ld	a5,-162(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc02010ea:	8522                	mv	a0,s0
ffffffffc02010ec:	6402                	ld	s0,0(sp)
ffffffffc02010ee:	60a2                	ld	ra,8(sp)
ffffffffc02010f0:	739c                	ld	a5,32(a5)
ffffffffc02010f2:	4585                	li	a1,1
ffffffffc02010f4:	0141                	addi	sp,sp,16
ffffffffc02010f6:	8782                	jr	a5
ffffffffc02010f8:	f8ffe0ef          	jal	ffffffffc0200086 <intr_disable>
ffffffffc02010fc:	00007797          	auipc	a5,0x7
ffffffffc0201100:	f447b783          	ld	a5,-188(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0201104:	739c                	ld	a5,32(a5)
ffffffffc0201106:	8522                	mv	a0,s0
ffffffffc0201108:	4585                	li	a1,1
ffffffffc020110a:	9782                	jalr	a5
ffffffffc020110c:	6402                	ld	s0,0(sp)
ffffffffc020110e:	60a2                	ld	ra,8(sp)
ffffffffc0201110:	0141                	addi	sp,sp,16
ffffffffc0201112:	f6ffe06f          	j	ffffffffc0200080 <intr_enable>
ffffffffc0201116:	9c5ff0ef          	jal	ffffffffc0200ada <pa2page.part.0>
ffffffffc020111a:	00002697          	auipc	a3,0x2
ffffffffc020111e:	56668693          	addi	a3,a3,1382 # ffffffffc0203680 <test_lab4_pmm_basic+0x838>
ffffffffc0201122:	00002617          	auipc	a2,0x2
ffffffffc0201126:	54660613          	addi	a2,a2,1350 # ffffffffc0203668 <test_lab4_pmm_basic+0x820>
ffffffffc020112a:	13b00593          	li	a1,315
ffffffffc020112e:	00002517          	auipc	a0,0x2
ffffffffc0201132:	33a50513          	addi	a0,a0,826 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0201136:	94bff0ef          	jal	ffffffffc0200a80 <__panic>
