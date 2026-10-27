/* Isolated foundation entry: real boot/DTB/SBI/PMM, no production Trap handlers. */
#include <assert.h>
#include <defs.h>
#include <dtb.h>
#include <intr.h>
#include <memlayout.h>
#include <mmu.h>
#include <pmm.h>
#include <riscv.h>
#include <sbi.h>
#include <stdio.h>
#include <string.h>

static void __noreturn __attribute__((aligned(4))) foundation_unexpected_trap(void) {
    cprintf("FOUNDATION_FAIL: unexpected trap cause=%lx epc=%lx\n",
            read_csr(scause), read_csr(sepc));
    while (1) asm volatile("");
}

/* Test-only panic reporting; production panic/monitor is outside this fixture. */
void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    clear_csr(sstatus, SSTATUS_SIE);
    cprintf("FOUNDATION_FAIL at %s:%d: ", file, line);
    va_list ap;
    va_start(ap, fmt);
    vcprintf(fmt, ap);
    va_end(ap);
    cprintf("\n");
    while (1) asm volatile("");
}

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    clear_csr(sstatus, SSTATUS_SIE);
    write_csr(sie, 0);
    write_csr(stvec, (uintptr_t)foundation_unexpected_trap);
    cprintf("FOUNDATION_C_ENTRY hart=%lu dtb=%lx\n", boot_hartid, boot_dtb);
    assert(boot_hartid == 0 && boot_dtb != 0);
    dtb_init();
    assert(get_memory_base() == DRAM_BASE);
    assert(get_memory_size() == 128UL * 1024 * 1024);
    pmm_init();
    assert(strcmp(pmm_manager->name, "best_fit_pmm_manager") == 0);
    assert(va_pa_offset == PHYSICAL_MEMORY_OFFSET);
    assert(npage == PPN(KERNTOP - PHYSICAL_MEMORY_OFFSET));
    assert((uintptr_t)pages == ROUNDUP((uintptr_t)end, PGSIZE));
    uintptr_t free_begin = ROUNDUP(PADDR(pages) +
                                (npage - nbase) * sizeof(struct Page), PGSIZE);
    size_t expected = (npage * PGSIZE - free_begin) / PGSIZE;
    size_t free_heads = 0, free_sum = 0;
    for (size_t i = 0; i < npage - nbase; i++) {
        struct Page *p = &pages[i];
        assert(page_ref(p) == 0);
        assert(PageReserved(p) == (page2pa(p) < free_begin));
        if (PageProperty(p)) {
            assert(!PageReserved(p));
            assert(page2pa(p) >= free_begin);
            assert(page2pa(p) + p->property * PGSIZE <= npage * PGSIZE);
            free_heads++;
            free_sum += p->property;
        }
    }
    assert(free_heads == 1 && free_sum == expected && nr_free_pages() == expected);
    for (unsigned enabled = 0; enabled <= 1; enabled++) {
        if (enabled) set_csr(sstatus, SSTATUS_SIE);
        else clear_csr(sstatus, SSTATUS_SIE);
        size_t count = nr_free_pages();
        assert(((read_csr(sstatus) & SSTATUS_SIE) != 0) == enabled);
        struct Page *p = alloc_pages(3);
        assert(p != NULL && page2pa(p) == free_begin);
        assert(((read_csr(sstatus) & SSTATUS_SIE) != 0) == enabled);
        assert(nr_free_pages() == count - 3);
        free_pages(p, 3);
        assert(((read_csr(sstatus) & SSTATUS_SIE) != 0) == enabled);
        assert(nr_free_pages() == count);
        assert(pa2page(free_begin)->property == expected);
    }
    clear_csr(sstatus, SSTATUS_SIE);
    assert(nr_free_pages() == expected);
    cprintf("FOUNDATION_LAYOUT npage=%lx nbase=%lx pages=%lx free_begin=%lx free_pages=%lu\n",
            npage, nbase, (uintptr_t)pages, free_begin, expected);
    cprintf("FOUNDATION_PASS: real entry, DTB, SBI, Best-Fit, reserved pages, SIE 0/1\n");
    sbi_shutdown();
    while (1) asm volatile("");
}
