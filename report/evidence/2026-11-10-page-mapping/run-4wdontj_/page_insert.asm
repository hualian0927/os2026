
/home/lhl/os2026/lab4/report/evidence/2026-11-10-page-mapping/checks-birpehib/software-O2/kernel:     file format elf64-littleriscv


Disassembly of section .text:

ffffffffc020113a <page_insert>:
ffffffffc020113a:	7179                	addi	sp,sp,-48
ffffffffc020113c:	ec26                	sd	s1,24(sp)
ffffffffc020113e:	84b2                	mv	s1,a2
ffffffffc0201140:	f022                	sd	s0,32(sp)
ffffffffc0201142:	4605                	li	a2,1
ffffffffc0201144:	842e                	mv	s0,a1
ffffffffc0201146:	85a6                	mv	a1,s1
ffffffffc0201148:	e84a                	sd	s2,16(sp)
ffffffffc020114a:	f406                	sd	ra,40(sp)
ffffffffc020114c:	8936                	mv	s2,a3
ffffffffc020114e:	c31ff0ef          	jal	ffffffffc0200d7e <get_pte>
ffffffffc0201152:	c159                	beqz	a0,ffffffffc02011d8 <page_insert+0x9e>
ffffffffc0201154:	611c                	ld	a5,0(a0)
ffffffffc0201156:	e44e                	sd	s3,8(sp)
ffffffffc0201158:	e052                	sd	s4,0(sp)
ffffffffc020115a:	0017f713          	andi	a4,a5,1
ffffffffc020115e:	89aa                	mv	s3,a0
ffffffffc0201160:	00007a17          	auipc	s4,0x7
ffffffffc0201164:	f08a0a13          	addi	s4,s4,-248 # ffffffffc0208068 <pages>
ffffffffc0201168:	e331                	bnez	a4,ffffffffc02011ac <page_insert+0x72>
ffffffffc020116a:	401c                	lw	a5,0(s0)
ffffffffc020116c:	85ce                	mv	a1,s3
ffffffffc020116e:	8526                	mv	a0,s1
ffffffffc0201170:	2785                	addiw	a5,a5,1
ffffffffc0201172:	c01c                	sw	a5,0(s0)
ffffffffc0201174:	f15ff0ef          	jal	ffffffffc0201088 <page_remove_pte.isra.0>
ffffffffc0201178:	000a3703          	ld	a4,0(s4)
ffffffffc020117c:	8c19                	sub	s0,s0,a4
ffffffffc020117e:	40645693          	srai	a3,s0,0x6
ffffffffc0201182:	000807b7          	lui	a5,0x80
ffffffffc0201186:	96be                	add	a3,a3,a5
ffffffffc0201188:	06aa                	slli	a3,a3,0xa
ffffffffc020118a:	0126e6b3          	or	a3,a3,s2
ffffffffc020118e:	0016e693          	ori	a3,a3,1
ffffffffc0201192:	00d9b023          	sd	a3,0(s3)
ffffffffc0201196:	12048073          	sfence.vma	s1
ffffffffc020119a:	69a2                	ld	s3,8(sp)
ffffffffc020119c:	6a02                	ld	s4,0(sp)
ffffffffc020119e:	4501                	li	a0,0
ffffffffc02011a0:	70a2                	ld	ra,40(sp)
ffffffffc02011a2:	7402                	ld	s0,32(sp)
ffffffffc02011a4:	64e2                	ld	s1,24(sp)
ffffffffc02011a6:	6942                	ld	s2,16(sp)
ffffffffc02011a8:	6145                	addi	sp,sp,48
ffffffffc02011aa:	8082                	ret
ffffffffc02011ac:	078a                	slli	a5,a5,0x2
ffffffffc02011ae:	83b1                	srli	a5,a5,0xc
ffffffffc02011b0:	00007717          	auipc	a4,0x7
ffffffffc02011b4:	eb073703          	ld	a4,-336(a4) # ffffffffc0208060 <npage>
ffffffffc02011b8:	02e7f263          	bgeu	a5,a4,ffffffffc02011dc <page_insert+0xa2>
ffffffffc02011bc:	00007a17          	auipc	s4,0x7
ffffffffc02011c0:	eaca0a13          	addi	s4,s4,-340 # ffffffffc0208068 <pages>
ffffffffc02011c4:	000a3703          	ld	a4,0(s4)
ffffffffc02011c8:	fff806b7          	lui	a3,0xfff80
ffffffffc02011cc:	97b6                	add	a5,a5,a3
ffffffffc02011ce:	079a                	slli	a5,a5,0x6
ffffffffc02011d0:	97ba                	add	a5,a5,a4
ffffffffc02011d2:	f8f41ce3          	bne	s0,a5,ffffffffc020116a <page_insert+0x30>
ffffffffc02011d6:	b75d                	j	ffffffffc020117c <page_insert+0x42>
ffffffffc02011d8:	5571                	li	a0,-4
ffffffffc02011da:	b7d9                	j	ffffffffc02011a0 <page_insert+0x66>
ffffffffc02011dc:	8ffff0ef          	jal	ffffffffc0200ada <pa2page.part.0>
