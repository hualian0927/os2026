/* Isolated Lab4 page-mapping check. Actual pmm.c, allocator and trapentry.S;
 * fixture kern_init/panic/unexpected-trap exit, no process/scheduler stubs. */
#include <pmm.c>
#include <console.h>
#include <dtb.h>
#include <intr.h>
#include <trap.h>

static const char *stage = "startup";
void test_lab4_pmm_basic(void);

void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    intr_disable();
    cprintf("MAPPING_FAIL stage=%s at %s:%d: ", stage, file, line);
    va_list ap;
    va_start(ap, fmt); vcprintf(fmt, ap); va_end(ap);
    cprintf("\n");
    sbi_shutdown();
    while (1) asm volatile("");
}

void trap(struct trapframe *tf) {
    intr_disable();
    cprintf("MAPPING_FAIL unexpected trap stage=%s cause=%lx epc=%lx address=%lx\n",
            stage, (unsigned long)tf->cause, (unsigned long)tf->epc,
            (unsigned long)tf->badvaddr);
    sbi_shutdown();
    while (1) asm volatile("");
}

/* Test-owned empty branches only. Production page_remove keeps these tables. */
static void release_empty_branch(unsigned index) {
    if (!(boot_pgdir_va[index] & PTE_V)) return;
    pte_t *middle = KADDR(PDE_ADDR(boot_pgdir_va[index]));
    for (unsigned i = 0; i < NPDEENTRY; i++) {
        if (!(middle[i] & PTE_V)) continue;
        pte_t *leaf = KADDR(PDE_ADDR(middle[i]));
        for (unsigned j = 0; j < NPTEENTRY; j++) assert(leaf[j] == 0);
        struct Page *p = pde2page(middle[i]);
        middle[i] = 0;
        flush_tlb();
        assert(page_ref_dec(p) == 0);
        free_page(p);
    }
    struct Page *p = pde2page(boot_pgdir_va[index]);
    boot_pgdir_va[index] = 0;
    flush_tlb();
    assert(page_ref_dec(p) == 0);
    free_page(p);
}

static void software_matrix(void) {
    stage = "software matrix";
    size_t initial = nr_free_pages();
    struct Page *a = alloc_page(), *b = alloc_page();
    assert(a && b && page_ref(a) == 0 && page_ref(b) == 0);
    assert(nr_free_pages() == initial - 2);
    assert(page_insert(boot_pgdir_va, a, 0x4100, PTE_R | PTE_W) == 0);
    assert(get_page(boot_pgdir_va, 0x4000, NULL) == a && page_ref(a) == 1);
    assert(nr_free_pages() == initial - 4);
    assert(page_insert(boot_pgdir_va, a, 0x4000, 0) == 0);
    pte_t *slot = get_pte(boot_pgdir_va, 0x4000, 0);
    assert((*slot & 0x3ff) == PTE_V && page_ref(a) == 1);
    assert(page_insert(boot_pgdir_va, a, 0x5000, PTE_U | PTE_W) == 0);
    assert(page_ref(a) == 2);
    assert(page_insert(boot_pgdir_va, b, 0x4000, PTE_R) == 0);
    assert(page_ref(a) == 1 && page_ref(b) == 1);
    assert(page_insert(boot_pgdir_va, b, 0x4000, PTE_R | PTE_W) == 0);
    assert(page_ref(b) == 1 && nr_free_pages() == initial - 4);
    page_remove(boot_pgdir_va, 0x5100);
    assert(page_ref(a) == 0 && nr_free_pages() == initial - 3);
    page_remove(boot_pgdir_va, 0x5000);
    assert(nr_free_pages() == initial - 3);
    page_remove(boot_pgdir_va, 0x4000);
    assert(page_ref(b) == 0 && nr_free_pages() == initial - 2);
    page_remove(boot_pgdir_va, 0x40000000);
    assert(boot_pgdir_va[1] == 0 && nr_free_pages() == initial - 2);
    release_empty_branch(0);
    assert(nr_free_pages() == initial);
    cprintf("PASS software: unaligned VA, exact requested flags, same-page remap, shared replacement, idempotent removal, page recovery\n");
}

static void allocation_failure(unsigned remaining, bool existing_parent) {
    stage = existing_parent ? "existing parent OOM" : "new parent OOM";
    size_t initial = nr_free_pages();
    struct Page *input = alloc_page(), *old = alloc_page();
    assert(input && old);
    uintptr_t old_va = existing_parent ? 0x40000000 : 0x4000;
    uintptr_t target = existing_parent ? 0x40200000 : 0x40000000;
    assert(page_insert(boot_pgdir_va, old, old_va, PTE_R | PTE_W) == 0);
    pte_t *old_slot = get_pte(boot_pgdir_va, old_va, 0);
    pte_t old_mapping = *old_slot;
    if (!existing_parent) boot_pgdir_va[1] = PTE_SOFT;
    pde_t old_parent = boot_pgdir_va[1];
    struct Page *parent = existing_parent ? pde2page(old_parent) : NULL;
    size_t held_count = nr_free_pages() - remaining;
    struct Page *held = alloc_pages(held_count);
    assert(held && nr_free_pages() == remaining);
    assert(page_insert(boot_pgdir_va, input, target, PTE_R | PTE_W) == -E_NO_MEM);
    assert(nr_free_pages() == remaining && page_ref(input) == 0);
    assert(boot_pgdir_va[1] == old_parent && *old_slot == old_mapping);
    assert(get_page(boot_pgdir_va, old_va, NULL) == old && page_ref(old) == 1);
    assert(get_pte(boot_pgdir_va, target, 0) == NULL);
    if (existing_parent) assert(page_ref(parent) == 1);
    free_pages(held, held_count);
    page_remove(boot_pgdir_va, old_va);
    free_page(input);
    release_empty_branch(existing_parent ? 1 : 0);
    if (!existing_parent) boot_pgdir_va[1] = 0;
    flush_tlb();
    assert(nr_free_pages() == initial);
    cprintf("PASS OOM: remaining=%u existing_parent=%d, old PTE/input ref preserved, new tables rolled back\n",
            remaining, existing_parent);
}

static void hardware_alias(void) {
    stage = "real MMU alias/replacement";
    size_t initial = nr_free_pages();
    struct Page *a = alloc_page(), *b = alloc_page();
    assert(a && b);
    *(volatile uintptr_t *)page2kva(a) = 0x123456789abcdef0ULL;
    *(volatile uintptr_t *)page2kva(b) = 0xfedcba9876543210ULL;
    volatile uintptr_t *x = (volatile uintptr_t *)0x4000;
    volatile uintptr_t *y = (volatile uintptr_t *)0x5000;
    assert(page_insert(boot_pgdir_va, a, (uintptr_t)x, PTE_R | PTE_W) == 0);
    assert(*x == 0x123456789abcdef0ULL); /* Warm the old translation. */
    assert(page_insert(boot_pgdir_va, a, (uintptr_t)y, PTE_R | PTE_W) == 0);
    *y = 0x1111222233334444ULL;
    assert(*x == 0x1111222233334444ULL);
    assert(page_insert(boot_pgdir_va, b, (uintptr_t)x, PTE_R | PTE_W) == 0);
    assert(*x == 0xfedcba9876543210ULL && *y == 0x1111222233334444ULL);
    *x = 0x5555666677778888ULL;
    assert(*(volatile uintptr_t *)page2kva(b) == 0x5555666677778888ULL);
    page_remove(boot_pgdir_va, (uintptr_t)y);
    page_remove(boot_pgdir_va, (uintptr_t)x);
    release_empty_branch(0);
    assert(nr_free_pages() == initial);
    cprintf("PASS hardware: actual aliases, warmed mapping replaced by another physical page, real reads/writes\n");
}

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    intr_disable(); write_csr(sie, 0); cons_init(); dtb_init();
    extern void __alltraps(void);
    write_csr(stvec, (uintptr_t)__alltraps); write_csr(sscratch, 0);
    init_pmm_manager(); page_init();
    extern char boot_page_table_sv39[];
    boot_pgdir_va = (pde_t *)boot_page_table_sv39;
    boot_pgdir_pa = PADDR(boot_pgdir_va);
    check_alloc_page();
    stage = "unchanged check_pgdir"; check_pgdir();
    stage = "unchanged tools PMM test"; test_lab4_pmm_basic();
    release_empty_branch(0);
    cprintf("PASS unchanged framework software checks\n");
    software_matrix();
    allocation_failure(0, 0);
    allocation_failure(1, 0);
    allocation_failure(0, 1);
    hardware_alias();
    stage = "unchanged check_boot_pgdir"; check_boot_pgdir();
    cprintf("MAPPING_PASS: actual implementation, original tests and bounded independent checks\n");
    sbi_shutdown();
    while (1) asm volatile("");
}
