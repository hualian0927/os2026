
/home/lhl/os2026/lab4/report/evidence/2026-11-10-page-mapping/checks-birpehib/software-O2/kernel:     file format elf64-littleriscv


Disassembly of section .text:

ffffffffc0200d7e <get_pte>:
ffffffffc0200d7e:	01e5d793          	srli	a5,a1,0x1e
ffffffffc0200d82:	1ff7f793          	andi	a5,a5,511
ffffffffc0200d86:	715d                	addi	sp,sp,-80
ffffffffc0200d88:	078e                	slli	a5,a5,0x3
ffffffffc0200d8a:	f84a                	sd	s2,48(sp)
ffffffffc0200d8c:	00f50933          	add	s2,a0,a5
ffffffffc0200d90:	ec56                	sd	s5,24(sp)
ffffffffc0200d92:	00093a83          	ld	s5,0(s2)
ffffffffc0200d96:	fc26                	sd	s1,56(sp)
ffffffffc0200d98:	f052                	sd	s4,32(sp)
ffffffffc0200d9a:	e486                	sd	ra,72(sp)
ffffffffc0200d9c:	e0a2                	sd	s0,64(sp)
ffffffffc0200d9e:	f44e                	sd	s3,40(sp)
ffffffffc0200da0:	e85a                	sd	s6,16(sp)
ffffffffc0200da2:	e45e                	sd	s7,8(sp)
ffffffffc0200da4:	001af793          	andi	a5,s5,1
ffffffffc0200da8:	84ae                	mv	s1,a1
ffffffffc0200daa:	8a32                	mv	s4,a2
ffffffffc0200dac:	e3d9                	bnez	a5,ffffffffc0200e32 <get_pte+0xb4>
ffffffffc0200dae:	14060163          	beqz	a2,ffffffffc0200ef0 <get_pte+0x172>
ffffffffc0200db2:	100027f3          	csrr	a5,sstatus
ffffffffc0200db6:	8b89                	andi	a5,a5,2
ffffffffc0200db8:	1a079563          	bnez	a5,ffffffffc0200f62 <get_pte+0x1e4>
ffffffffc0200dbc:	00007797          	auipc	a5,0x7
ffffffffc0200dc0:	2847b783          	ld	a5,644(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200dc4:	6f9c                	ld	a5,24(a5)
ffffffffc0200dc6:	4505                	li	a0,1
ffffffffc0200dc8:	9782                	jalr	a5
ffffffffc0200dca:	89aa                	mv	s3,a0
ffffffffc0200dcc:	12098263          	beqz	s3,ffffffffc0200ef0 <get_pte+0x172>
ffffffffc0200dd0:	00007b97          	auipc	s7,0x7
ffffffffc0200dd4:	298b8b93          	addi	s7,s7,664 # ffffffffc0208068 <pages>
ffffffffc0200dd8:	000bb503          	ld	a0,0(s7)
ffffffffc0200ddc:	00080437          	lui	s0,0x80
ffffffffc0200de0:	00007b17          	auipc	s6,0x7
ffffffffc0200de4:	280b0b13          	addi	s6,s6,640 # ffffffffc0208060 <npage>
ffffffffc0200de8:	40a98533          	sub	a0,s3,a0
ffffffffc0200dec:	8519                	srai	a0,a0,0x6
ffffffffc0200dee:	9522                	add	a0,a0,s0
ffffffffc0200df0:	000b3703          	ld	a4,0(s6)
ffffffffc0200df4:	00c51793          	slli	a5,a0,0xc
ffffffffc0200df8:	4685                	li	a3,1
ffffffffc0200dfa:	00d9a023          	sw	a3,0(s3)
ffffffffc0200dfe:	83b1                	srli	a5,a5,0xc
ffffffffc0200e00:	0532                	slli	a0,a0,0xc
ffffffffc0200e02:	20e7f063          	bgeu	a5,a4,ffffffffc0201002 <get_pte+0x284>
ffffffffc0200e06:	00007797          	auipc	a5,0x7
ffffffffc0200e0a:	2527b783          	ld	a5,594(a5) # ffffffffc0208058 <va_pa_offset>
ffffffffc0200e0e:	4581                	li	a1,0
ffffffffc0200e10:	6605                	lui	a2,0x1
ffffffffc0200e12:	953e                	add	a0,a0,a5
ffffffffc0200e14:	f64ff0ef          	jal	ffffffffc0200578 <memset>
ffffffffc0200e18:	000bb683          	ld	a3,0(s7)
ffffffffc0200e1c:	4585                	li	a1,1
ffffffffc0200e1e:	40d986b3          	sub	a3,s3,a3
ffffffffc0200e22:	8699                	srai	a3,a3,0x6
ffffffffc0200e24:	96a2                	add	a3,a3,s0
ffffffffc0200e26:	06aa                	slli	a3,a3,0xa
ffffffffc0200e28:	0116e693          	ori	a3,a3,17
ffffffffc0200e2c:	00d93023          	sd	a3,0(s2)
ffffffffc0200e30:	a801                	j	ffffffffc0200e40 <get_pte+0xc2>
ffffffffc0200e32:	86d6                	mv	a3,s5
ffffffffc0200e34:	4581                	li	a1,0
ffffffffc0200e36:	4981                	li	s3,0
ffffffffc0200e38:	00007b17          	auipc	s6,0x7
ffffffffc0200e3c:	228b0b13          	addi	s6,s6,552 # ffffffffc0208060 <npage>
ffffffffc0200e40:	77fd                	lui	a5,0xfffff
ffffffffc0200e42:	068a                	slli	a3,a3,0x2
ffffffffc0200e44:	000b3703          	ld	a4,0(s6)
ffffffffc0200e48:	8efd                	and	a3,a3,a5
ffffffffc0200e4a:	00c6d793          	srli	a5,a3,0xc
ffffffffc0200e4e:	16e7f263          	bgeu	a5,a4,ffffffffc0200fb2 <get_pte+0x234>
ffffffffc0200e52:	00007b97          	auipc	s7,0x7
ffffffffc0200e56:	206b8b93          	addi	s7,s7,518 # ffffffffc0208058 <va_pa_offset>
ffffffffc0200e5a:	000bb603          	ld	a2,0(s7)
ffffffffc0200e5e:	0154d793          	srli	a5,s1,0x15
ffffffffc0200e62:	1ff7f793          	andi	a5,a5,511
ffffffffc0200e66:	96b2                	add	a3,a3,a2
ffffffffc0200e68:	078e                	slli	a5,a5,0x3
ffffffffc0200e6a:	00f68433          	add	s0,a3,a5
ffffffffc0200e6e:	6014                	ld	a3,0(s0)
ffffffffc0200e70:	0016f793          	andi	a5,a3,1
ffffffffc0200e74:	c3b1                	beqz	a5,ffffffffc0200eb8 <get_pte+0x13a>
ffffffffc0200e76:	c591                	beqz	a1,ffffffffc0200e82 <get_pte+0x104>
ffffffffc0200e78:	12000073          	sfence.vma
ffffffffc0200e7c:	6014                	ld	a3,0(s0)
ffffffffc0200e7e:	000b3703          	ld	a4,0(s6)
ffffffffc0200e82:	77fd                	lui	a5,0xfffff
ffffffffc0200e84:	068a                	slli	a3,a3,0x2
ffffffffc0200e86:	8efd                	and	a3,a3,a5
ffffffffc0200e88:	00c6d793          	srli	a5,a3,0xc
ffffffffc0200e8c:	12e7ff63          	bgeu	a5,a4,ffffffffc0200fca <get_pte+0x24c>
ffffffffc0200e90:	000bb783          	ld	a5,0(s7)
ffffffffc0200e94:	80b1                	srli	s1,s1,0xc
ffffffffc0200e96:	1ff4f493          	andi	s1,s1,511
ffffffffc0200e9a:	96be                	add	a3,a3,a5
ffffffffc0200e9c:	048e                	slli	s1,s1,0x3
ffffffffc0200e9e:	00968533          	add	a0,a3,s1
ffffffffc0200ea2:	60a6                	ld	ra,72(sp)
ffffffffc0200ea4:	6406                	ld	s0,64(sp)
ffffffffc0200ea6:	74e2                	ld	s1,56(sp)
ffffffffc0200ea8:	7942                	ld	s2,48(sp)
ffffffffc0200eaa:	79a2                	ld	s3,40(sp)
ffffffffc0200eac:	7a02                	ld	s4,32(sp)
ffffffffc0200eae:	6ae2                	ld	s5,24(sp)
ffffffffc0200eb0:	6b42                	ld	s6,16(sp)
ffffffffc0200eb2:	6ba2                	ld	s7,8(sp)
ffffffffc0200eb4:	6161                	addi	sp,sp,80
ffffffffc0200eb6:	8082                	ret
ffffffffc0200eb8:	020a1e63          	bnez	s4,ffffffffc0200ef4 <get_pte+0x176>
ffffffffc0200ebc:	02098a63          	beqz	s3,ffffffffc0200ef0 <get_pte+0x172>
ffffffffc0200ec0:	01593023          	sd	s5,0(s2)
ffffffffc0200ec4:	12000073          	sfence.vma
ffffffffc0200ec8:	0009a783          	lw	a5,0(s3)
ffffffffc0200ecc:	fff7871b          	addiw	a4,a5,-1 # ffffffffffffefff <end+0x3fdf6f8f>
ffffffffc0200ed0:	00e9a023          	sw	a4,0(s3)
ffffffffc0200ed4:	10071763          	bnez	a4,ffffffffc0200fe2 <get_pte+0x264>
ffffffffc0200ed8:	100027f3          	csrr	a5,sstatus
ffffffffc0200edc:	8b89                	andi	a5,a5,2
ffffffffc0200ede:	efd9                	bnez	a5,ffffffffc0200f7c <get_pte+0x1fe>
ffffffffc0200ee0:	00007797          	auipc	a5,0x7
ffffffffc0200ee4:	1607b783          	ld	a5,352(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200ee8:	739c                	ld	a5,32(a5)
ffffffffc0200eea:	4585                	li	a1,1
ffffffffc0200eec:	854e                	mv	a0,s3
ffffffffc0200eee:	9782                	jalr	a5
ffffffffc0200ef0:	4501                	li	a0,0
ffffffffc0200ef2:	bf45                	j	ffffffffc0200ea2 <get_pte+0x124>
ffffffffc0200ef4:	100027f3          	csrr	a5,sstatus
ffffffffc0200ef8:	8b89                	andi	a5,a5,2
ffffffffc0200efa:	efd9                	bnez	a5,ffffffffc0200f98 <get_pte+0x21a>
ffffffffc0200efc:	00007797          	auipc	a5,0x7
ffffffffc0200f00:	1447b783          	ld	a5,324(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200f04:	6f9c                	ld	a5,24(a5)
ffffffffc0200f06:	4505                	li	a0,1
ffffffffc0200f08:	9782                	jalr	a5
ffffffffc0200f0a:	8a2a                	mv	s4,a0
ffffffffc0200f0c:	fa0a08e3          	beqz	s4,ffffffffc0200ebc <get_pte+0x13e>
ffffffffc0200f10:	00007997          	auipc	s3,0x7
ffffffffc0200f14:	15898993          	addi	s3,s3,344 # ffffffffc0208068 <pages>
ffffffffc0200f18:	0009b683          	ld	a3,0(s3)
ffffffffc0200f1c:	00080937          	lui	s2,0x80
ffffffffc0200f20:	000b3703          	ld	a4,0(s6)
ffffffffc0200f24:	40da06b3          	sub	a3,s4,a3
ffffffffc0200f28:	8699                	srai	a3,a3,0x6
ffffffffc0200f2a:	96ca                	add	a3,a3,s2
ffffffffc0200f2c:	00c69793          	slli	a5,a3,0xc
ffffffffc0200f30:	4605                	li	a2,1
ffffffffc0200f32:	00ca2023          	sw	a2,0(s4)
ffffffffc0200f36:	83b1                	srli	a5,a5,0xc
ffffffffc0200f38:	06b2                	slli	a3,a3,0xc
ffffffffc0200f3a:	0ee7f163          	bgeu	a5,a4,ffffffffc020101c <get_pte+0x29e>
ffffffffc0200f3e:	000bb503          	ld	a0,0(s7)
ffffffffc0200f42:	6605                	lui	a2,0x1
ffffffffc0200f44:	4581                	li	a1,0
ffffffffc0200f46:	9536                	add	a0,a0,a3
ffffffffc0200f48:	e30ff0ef          	jal	ffffffffc0200578 <memset>
ffffffffc0200f4c:	0009b783          	ld	a5,0(s3)
ffffffffc0200f50:	40fa07b3          	sub	a5,s4,a5
ffffffffc0200f54:	8799                	srai	a5,a5,0x6
ffffffffc0200f56:	97ca                	add	a5,a5,s2
ffffffffc0200f58:	07aa                	slli	a5,a5,0xa
ffffffffc0200f5a:	0117e793          	ori	a5,a5,17
ffffffffc0200f5e:	e01c                	sd	a5,0(s0)
ffffffffc0200f60:	bf21                	j	ffffffffc0200e78 <get_pte+0xfa>
ffffffffc0200f62:	924ff0ef          	jal	ffffffffc0200086 <intr_disable>
ffffffffc0200f66:	00007797          	auipc	a5,0x7
ffffffffc0200f6a:	0da7b783          	ld	a5,218(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200f6e:	6f9c                	ld	a5,24(a5)
ffffffffc0200f70:	4505                	li	a0,1
ffffffffc0200f72:	9782                	jalr	a5
ffffffffc0200f74:	89aa                	mv	s3,a0
ffffffffc0200f76:	90aff0ef          	jal	ffffffffc0200080 <intr_enable>
ffffffffc0200f7a:	bd89                	j	ffffffffc0200dcc <get_pte+0x4e>
ffffffffc0200f7c:	90aff0ef          	jal	ffffffffc0200086 <intr_disable>
ffffffffc0200f80:	00007797          	auipc	a5,0x7
ffffffffc0200f84:	0c07b783          	ld	a5,192(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200f88:	739c                	ld	a5,32(a5)
ffffffffc0200f8a:	854e                	mv	a0,s3
ffffffffc0200f8c:	4585                	li	a1,1
ffffffffc0200f8e:	9782                	jalr	a5
ffffffffc0200f90:	8f0ff0ef          	jal	ffffffffc0200080 <intr_enable>
ffffffffc0200f94:	4501                	li	a0,0
ffffffffc0200f96:	b731                	j	ffffffffc0200ea2 <get_pte+0x124>
ffffffffc0200f98:	8eeff0ef          	jal	ffffffffc0200086 <intr_disable>
ffffffffc0200f9c:	00007797          	auipc	a5,0x7
ffffffffc0200fa0:	0a47b783          	ld	a5,164(a5) # ffffffffc0208040 <pmm_manager>
ffffffffc0200fa4:	6f9c                	ld	a5,24(a5)
ffffffffc0200fa6:	4505                	li	a0,1
ffffffffc0200fa8:	9782                	jalr	a5
ffffffffc0200faa:	8a2a                	mv	s4,a0
ffffffffc0200fac:	8d4ff0ef          	jal	ffffffffc0200080 <intr_enable>
ffffffffc0200fb0:	bfb1                	j	ffffffffc0200f0c <get_pte+0x18e>
ffffffffc0200fb2:	00002617          	auipc	a2,0x2
ffffffffc0200fb6:	66e60613          	addi	a2,a2,1646 # ffffffffc0203620 <test_lab4_pmm_basic+0x7d8>
ffffffffc0200fba:	10800593          	li	a1,264
ffffffffc0200fbe:	00002517          	auipc	a0,0x2
ffffffffc0200fc2:	4aa50513          	addi	a0,a0,1194 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0200fc6:	abbff0ef          	jal	ffffffffc0200a80 <__panic>
ffffffffc0200fca:	00002617          	auipc	a2,0x2
ffffffffc0200fce:	65660613          	addi	a2,a2,1622 # ffffffffc0203620 <test_lab4_pmm_basic+0x7d8>
ffffffffc0200fd2:	12200593          	li	a1,290
ffffffffc0200fd6:	00002517          	auipc	a0,0x2
ffffffffc0200fda:	49250513          	addi	a0,a0,1170 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0200fde:	aa3ff0ef          	jal	ffffffffc0200a80 <__panic>
ffffffffc0200fe2:	00002697          	auipc	a3,0x2
ffffffffc0200fe6:	66668693          	addi	a3,a3,1638 # ffffffffc0203648 <test_lab4_pmm_basic+0x800>
ffffffffc0200fea:	00002617          	auipc	a2,0x2
ffffffffc0200fee:	67e60613          	addi	a2,a2,1662 # ffffffffc0203668 <test_lab4_pmm_basic+0x820>
ffffffffc0200ff2:	11300593          	li	a1,275
ffffffffc0200ff6:	00002517          	auipc	a0,0x2
ffffffffc0200ffa:	47250513          	addi	a0,a0,1138 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0200ffe:	a83ff0ef          	jal	ffffffffc0200a80 <__panic>
ffffffffc0201002:	86aa                	mv	a3,a0
ffffffffc0201004:	00002617          	auipc	a2,0x2
ffffffffc0201008:	61c60613          	addi	a2,a2,1564 # ffffffffc0203620 <test_lab4_pmm_basic+0x7d8>
ffffffffc020100c:	10300593          	li	a1,259
ffffffffc0201010:	00002517          	auipc	a0,0x2
ffffffffc0201014:	45850513          	addi	a0,a0,1112 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0201018:	a69ff0ef          	jal	ffffffffc0200a80 <__panic>
ffffffffc020101c:	00002617          	auipc	a2,0x2
ffffffffc0201020:	60460613          	addi	a2,a2,1540 # ffffffffc0203620 <test_lab4_pmm_basic+0x7d8>
ffffffffc0201024:	11a00593          	li	a1,282
ffffffffc0201028:	00002517          	auipc	a0,0x2
ffffffffc020102c:	44050513          	addi	a0,a0,1088 # ffffffffc0203468 <test_lab4_pmm_basic+0x620>
ffffffffc0201030:	a51ff0ef          	jal	ffffffffc0200a80 <__panic>
