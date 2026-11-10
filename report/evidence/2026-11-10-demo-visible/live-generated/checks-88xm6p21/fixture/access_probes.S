/* Ordinary leaf calls obeying RV64 ABI; actual faults enter production
 * trapentry.S, whose C target is the explicitly isolated test observer. */
    .text
    .option push
    .option norvc
    .balign 4
    .globl mapping_probe_store
    .globl mapping_store_fault
    .globl mapping_store_resume
mapping_probe_store:
mapping_store_fault:
    sd a1, 0(a0)
mapping_store_resume:
    ret

    .balign 4
    .globl mapping_probe_load
    .globl mapping_load_fault
    .globl mapping_load_resume
mapping_probe_load:
mapping_load_fault:
    ld a0, 0(a0)
mapping_load_resume:
    ret
    .option pop
