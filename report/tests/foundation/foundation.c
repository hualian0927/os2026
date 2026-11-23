/* Independent foundation fixture. Includes the ACTUAL production pmm.c so
 * static page_init/check_alloc_page are tested without inventing Lab5 modules.
 * Unused pmm_init/page-table checks are removed by --gc-sections. */
#include <pmm.c>
#include <console.h>

#include <dtb.h>
#include <intr.h>
#include <trap.h>
extern void __alltraps(void);

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

static const struct pmm_manager *real_manager;
static unsigned allocation_budget;
static struct Page *limited_alloc(size_t n) {
    if (allocation_budget == 0) return NULL;
    allocation_budget--;
    return real_manager->alloc_pages(n);
}
static void check_walk_failures(void) {
    size_t before = nr_free_pages();
    struct Page *root_page = alloc_page();
    assert(root_page != NULL);
    pde_t *root = page2kva(root_page);
    memset(root,0,PGSIZE);
    size_t root_only = nr_free_pages();
    real_manager = pmm_manager;
    struct pmm_manager limited = *real_manager;
    limited.alloc_pages = limited_alloc;
    pmm_manager = &limited;
    for (unsigned allowed=0;allowed<2;allowed++) {
        allocation_budget = allowed;
        assert(get_pte(root,0x400000,1)==NULL);
        assert(root[0]==0 && nr_free_pages()==root_only);
    }
    pmm_manager = real_manager;
    struct Page *mid_page = alloc_page();
    assert(mid_page != NULL);
    memset(page2kva(mid_page),0,PGSIZE);
    set_page_ref(mid_page,1);
    root[0] = ptd_create(page2ppn(mid_page));
    pde_t original = root[0];
    size_t existing = nr_free_pages();
    pmm_manager = &limited; allocation_budget=0;
    assert(get_pte(root,0x400000,1)==NULL);
    assert(root[0]==original && page_ref(mid_page)==1 && nr_free_pages()==existing);
    pmm_manager = real_manager;
    root[0]=0; free_page(mid_page); free_page(root_page);
    assert(nr_free_pages()==before);
    cprintf("PASS get_pte failures: first/second allocation, new-table rollback, existing-table preservation\n");
}

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    intr_disable();
    write_csr(sie, 0);
    cons_init();
    write_csr(stvec, (uintptr_t)__alltraps);
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
        check_alloc_page(); /* Unchanged target basic_check and default_check. */
        assert(nr_free_pages() == expected_free);
        struct Page *p = alloc_pages(3);
        assert(p && page2pa(p) == free_begin && nr_free_pages() == expected_free - 3);
        free_page(p + 1); free_page(p); free_page(p + 2);
        assert(nr_free_pages() == expected_free);
        assert(alloc_pages(3) == p);
        free_pages(p, 3);
        assert(nr_free_pages() == expected_free);
        cprintf("PASS manager: unchanged target tests, split/coalesce, full page-count recovery\n");
        struct Page *block = alloc_pages(12);
        assert(block != NULL);
        free_pages(block + 1, 4); free_pages(block + 8, 2);
        assert(alloc_page() == block + 8);
        assert(alloc_pages(3) == block + 1);
        assert(alloc_page() == block + 4);
        assert(alloc_page() == block + 9);
        free_pages(block,12);
        assert(nr_free_pages() == expected_free);
        cprintf("PASS actual Best-Fit: smallest-fit and low-address tie\n");
        check_walk_failures();
        check_interrupt_wrappers();
        extern char boot_page_table_sv39[];
        boot_pgdir_va = (pte_t *)boot_page_table_sv39;
        boot_pgdir_pa = PADDR(boot_pgdir_va);
        check_pgdir(); /* Actual unchanged target metadata assertions. */
        assert(nr_free_pages() == expected_free);
#ifdef RUN_FRAMEWORK_BOOT_CHECK
        check_boot_pgdir(); /* Preserved; observe architecture conflict, do not patch. */
#endif
        cprintf("FOUNDATION_PASS: real DTB, descriptors, allocator, original metadata tests, SIE wrappers\n");
    }
    sbi_shutdown();
    while (1) asm volatile("");
}

/* Controlled test trap only; production exception/syscall modules remain absent. */
void trap(struct trapframe *tf) {
#ifdef RUN_FRAMEWORK_BOOT_CHECK
    assert(tf->cause == 15 && tf->tval == 0x100);
    assert(boot_pgdir_va[0] & PTE_U);
    cprintf("FRAMEWORK_CONFLICT_OBSERVED: unchanged check_pgdir expects non-leaf U; real store faults cause=%lu tval=%lx\n",tf->cause,tf->tval);
    sbi_shutdown(); while(1){}
#else
    panic("unexpected fixture trap cause=%lu epc=%lx tval=%lx",tf->cause,tf->epc,tf->tval);
#endif
}
