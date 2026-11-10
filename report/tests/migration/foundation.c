/* Independent foundation fixture. Includes the ACTUAL production pmm.c so
 * static page_init/check_alloc_page are tested without inventing Lab4 modules.
 * Unused pmm_init/page-table checks are removed by --gc-sections. */
#include <pmm.c>
#include <console.h>
#include <clock.h>
#include <dtb.h>
#include <intr.h>
#include <trap.h>

#ifndef FIXTURE_CASE
#define FIXTURE_CASE 0
#endif

#if FIXTURE_CASE != 0
/* Isolated DTB getter inputs, NOT the production device-tree parser. */
uint64_t get_memory_base(void) { return FIXTURE_CASE == 1 ? 0 : DRAM_BASE; }
uint64_t get_memory_size(void) {
    extern char end[];
    uintptr_t descriptors = ROUNDUP((uintptr_t)end - PHYSICAL_MEMORY_OFFSET, PGSIZE);
    if (FIXTURE_CASE == 2) return 0;
    if (FIXTURE_CASE == 3) return (uint64_t)-1;
    if (FIXTURE_CASE == 4) return descriptors + PGSIZE - DRAM_BASE;
    if (FIXTURE_CASE == 5) return 0x100000;
    /* Fixed point: descriptors fit, but their page-aligned tail reaches end. */
    uintptr_t upper = descriptors + 8 * PGSIZE;
    for (unsigned i = 0; i < 32; i++) {
        uintptr_t next = descriptors + ROUNDUP((PPN(upper) - nbase) * sizeof(struct Page), PGSIZE);
        if (next == upper) return upper - DRAM_BASE;
        upper = next;
    }
    return 0;
}
#endif

void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    static const char *expected[] = {
        "", "page_init: invalid device-tree memory range",
        "page_init: invalid device-tree memory range",
        "page_init: device-tree memory range overflows",
        "page_init: insufficient memory for page descriptors",
        "page_init: kernel is outside the usable memory range", ""
    };
    clear_csr(sstatus, SSTATUS_SIE);
    cprintf("FIXTURE panic at %s:%d: ", file, line);
    va_list ap;
    va_start(ap, fmt); vcprintf(fmt, ap); va_end(ap);
    cprintf("\n");
    if (FIXTURE_CASE > 0 && FIXTURE_CASE < 6 && !strcmp(fmt, expected[FIXTURE_CASE]))
        cprintf("EXPECTED_PANIC_PASS case=%d\n", FIXTURE_CASE);
    else cprintf("FOUNDATION_FAIL case=%d\n", FIXTURE_CASE);
    sbi_shutdown();
    while (1) asm volatile("");
}

static void check_interrupt_wrappers(void) {
    /* Real production trap entry is installed; no asynchronous source enabled. */
    write_csr(sie, 0);
    for (unsigned enabled = 0; enabled < 2; enabled++) {
        if (enabled) intr_enable(); else intr_disable();
        bool outer, inner;
        local_intr_save(outer);
        assert(outer == enabled && !(read_csr(sstatus) & SSTATUS_SIE));
        local_intr_save(inner);
        assert(inner == 0);
        local_intr_restore(inner);
        assert(!(read_csr(sstatus) & SSTATUS_SIE));
        local_intr_restore(outer);
        assert(!!(read_csr(sstatus) & SSTATUS_SIE) == enabled);
        size_t before = nr_free_pages();
        assert(!!(read_csr(sstatus) & SSTATUS_SIE) == enabled);
        struct Page *p = alloc_page();
        assert(p != NULL && !!(read_csr(sstatus) & SSTATUS_SIE) == enabled);
        free_page(p);
        assert(!!(read_csr(sstatus) & SSTATUS_SIE) == enabled);
        assert(nr_free_pages() == before);
        assert(!!(read_csr(sstatus) & SSTATUS_SIE) == enabled);
    }
    intr_disable();
    cprintf("PASS interrupt wrappers: SIE=0/1, nested flags, alloc/free/count restore original state\n");
}

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    intr_disable();
    write_csr(sie, 0);
    cons_init();
    idt_init();
#if FIXTURE_CASE == 0
    dtb_init();
#else
    cprintf("ISOLATED GETTER FIXTURE case=%d; not production DTB parsing\n", FIXTURE_CASE);
#endif
    init_pmm_manager();
    page_init();
    assert(FIXTURE_CASE == 0 || FIXTURE_CASE == 6);
    assert(va_pa_offset == PHYSICAL_MEMORY_OFFSET);
    uintptr_t limit = KERNTOP - PHYSICAL_MEMORY_OFFSET;
    uintptr_t reported_end = get_memory_base() + get_memory_size();
    uintptr_t managed = ROUNDDOWN(reported_end < limit ? reported_end : limit, PGSIZE);
    uintptr_t descriptors = ROUNDUP((uintptr_t)end - PHYSICAL_MEMORY_OFFSET, PGSIZE);
    assert(npage == PPN(managed));
    assert((uintptr_t)pages == descriptors + va_pa_offset);
    size_t count = npage - nbase;
    uintptr_t free_begin = ROUNDUP(descriptors + count * sizeof(struct Page), PGSIZE);
    size_t expected_free = free_begin < managed ? (managed - free_begin) / PGSIZE : 0;
    for (size_t i = 0; i < count; i++) {
        struct Page *p = &pages[i];
        assert(PageReserved(p) == (page2pa(p) < free_begin));
        assert(page_ref(p) == 0);
        assert(p->pra_vaddr == 0 && list_empty(&p->pra_page_link));
    }
    assert(nr_free_pages() == expected_free);
    cprintf("PASS descriptors: sizeof(Page)=%lu count=%lu npage=%lx nbase=%lx\n",
            (unsigned long)sizeof(struct Page), (unsigned long)count,
            (unsigned long)npage, (unsigned long)nbase);
    cprintf("PASS reserved/free: kernel_end=%lx descriptors=%lx free_begin=%lx managed_end=%lx free_pages=%lu\n",
            (unsigned long)PADDR(end), (unsigned long)descriptors,
            (unsigned long)free_begin, (unsigned long)managed, (unsigned long)expected_free);
    if (FIXTURE_CASE == 6) {
        assert(expected_free == 0 && alloc_page() == NULL);
        cprintf("NO_FREE_PAGE_PASS: no PageProperty heads, no zero-length registration\n");
    } else {
        assert(expected_free > 3);
        check_alloc_page(); /* Unchanged Lab4 basic_check and best_fit_check. */
        assert(nr_free_pages() == expected_free);
        struct Page *p = alloc_pages(3);
        assert(p && page2pa(p) == free_begin && nr_free_pages() == expected_free - 3);
        free_page(p + 1); free_page(p); free_page(p + 2);
        assert(nr_free_pages() == expected_free);
        assert(alloc_pages(3) == p);
        free_pages(p, 3);
        assert(nr_free_pages() == expected_free);
        cprintf("PASS manager: Best-Fit framework tests, split/coalesce, full page-count recovery\n");
        check_interrupt_wrappers();
        uintptr_t before_sp;
        asm volatile("mv %0, sp" : "=r"(before_sp));
        asm volatile("c.ebreak\n .4byte 0x00100073\n .4byte 0x0000000b" ::: "memory");
        uintptr_t after_sp;
        asm volatile("mv %0, sp" : "=r"(after_sp));
        assert(before_sp == after_sp);
        cprintf("PASS production handlers: 16/32-bit breakpoint and 32-bit illegal instruction returned\n");
        clock_init();
        intr_enable();
        while (ticks < 102) asm volatile("" ::: "memory");
        intr_disable();
        write_csr(sie, 0);
        assert(ticks >= 102 && nr_free_pages() == expected_free);
        cprintf("PASS production timer: >=102 interrupts, rearmed and returned; no fixture handler\n");
        cprintf("FOUNDATION_PASS: real DTB/allocator/wrappers/trap/handlers; isolated kern_init\n");
    }
    sbi_shutdown();
    while (1) asm volatile("");
}
