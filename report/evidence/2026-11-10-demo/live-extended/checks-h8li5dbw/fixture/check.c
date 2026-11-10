/* Isolated Lab4 page-mapping check. Actual pmm.c, allocator and trapentry.S;
 * fixture kern_init/panic/unexpected-trap exit, no process/scheduler stubs. */
#include <pmm.c>
#include <console.h>
#include <dtb.h>
#include <intr.h>
#include <trap.h>

static const char *stage = "startup";
void test_lab4_pmm_basic(void);
#ifndef CHECK_MODE
#define CHECK_MODE 0
#endif

static volatile unsigned expected_fault_armed, observed_faults;
static uintptr_t expected_cause, expected_address, expected_epc, expected_resume;
void mapping_probe_store(uintptr_t address, uintptr_t value);
uintptr_t mapping_probe_load(uintptr_t address);
extern void mapping_store_fault(void), mapping_store_resume(void);
extern void mapping_load_fault(void), mapping_load_resume(void);

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
    if (expected_fault_armed) {
        assert(tf->cause == expected_cause && tf->badvaddr == expected_address);
        assert(tf->epc == expected_epc);
        assert((tf->status & SSTATUS_SPP) && !(tf->status & SSTATUS_SIE));
        observed_faults++;
        expected_fault_armed = 0;
        tf->epc = expected_resume;
        cprintf("OBSERVED expected fault: cause=%lu address=%lx epc=%lx\n",
                (unsigned long)tf->cause, (unsigned long)tf->badvaddr,
                (unsigned long)expected_epc);
        return;
    }
    cprintf("MAPPING_FAIL unexpected trap stage=%s cause=%lx epc=%lx address=%lx\n",
            stage, (unsigned long)tf->cause, (unsigned long)tf->epc,
            (unsigned long)tf->badvaddr);
    if (boot_pgdir_va && tf->badvaddr < 0x80000000) {
        pte_t l2 = boot_pgdir_va[PDX1(tf->badvaddr)];
        cprintf("PTE L2=%lx\n", (unsigned long)l2);
        if ((l2 & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V) {
            pte_t l1 = ((pte_t *)KADDR(PDE_ADDR(l2)))[PDX0(tf->badvaddr)];
            cprintf("PTE L1=%lx\n", (unsigned long)l1);
            if ((l1 & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V)
                cprintf("PTE L0=%lx\n", (unsigned long)((pte_t *)KADDR(PDE_ADDR(l1)))[PTX(tf->badvaddr)]);
        }
    }
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

static void allocation_failure(unsigned remaining, bool existing_parent, bool mapped_input) {
    stage = existing_parent ? "existing parent OOM" : "new parent OOM";
    size_t initial = nr_free_pages();
    struct Page *input = alloc_page(), *old = alloc_page();
    assert(input && old);
    uintptr_t old_va = existing_parent ? 0x40000000 : 0x4000;
    uintptr_t target = existing_parent ? 0x40200000 : 0x40000000;
    assert(page_insert(boot_pgdir_va, old, old_va, PTE_R | PTE_W) == 0);
    pte_t *old_slot = get_pte(boot_pgdir_va, old_va, 0);
    pte_t old_mapping = *old_slot;
    pte_t *input_slot = NULL;
    pte_t input_mapping = 0;
    if (mapped_input) {
        assert(page_insert(boot_pgdir_va, input, old_va + PGSIZE, PTE_R) == 0);
        input_slot = get_pte(boot_pgdir_va, old_va + PGSIZE, 0);
        input_mapping = *input_slot;
    }
    if (!existing_parent) boot_pgdir_va[1] = PTE_SOFT;
    pde_t old_parent = boot_pgdir_va[1];
    struct Page *parent = existing_parent ? pde2page(old_parent) : NULL;
    size_t held_count = nr_free_pages() - remaining;
    struct Page *held = alloc_pages(held_count);
    assert(held && nr_free_pages() == remaining);
    assert(page_insert(boot_pgdir_va, input, target, PTE_R | PTE_W) == -E_NO_MEM);
    assert(nr_free_pages() == remaining && page_ref(input) == (mapped_input ? 1 : 0));
    if (mapped_input) assert(*input_slot == input_mapping);
    assert(boot_pgdir_va[1] == old_parent && *old_slot == old_mapping);
    assert(get_page(boot_pgdir_va, old_va, NULL) == old && page_ref(old) == 1);
    assert(get_pte(boot_pgdir_va, target, 0) == NULL);
    if (existing_parent) assert(page_ref(parent) == 1);
    free_pages(held, held_count);
    page_remove(boot_pgdir_va, old_va);
    if (mapped_input) page_remove(boot_pgdir_va, old_va + PGSIZE);
    else free_page(input);
    release_empty_branch(existing_parent ? 1 : 0);
    if (!existing_parent) boot_pgdir_va[1] = 0;
    flush_tlb();
    assert(nr_free_pages() == initial);
    cprintf("PASS OOM: remaining=%u existing_parent=%d mapped_input=%d, old PTE/input ref preserved, new tables rolled back\n",
            remaining, existing_parent, mapped_input);
}

static void permission_matrix(void) {
    stage = "same-page permission matrix";
    const uint32_t permissions[] = {PTE_R | PTE_W | PTE_U | PTE_SOFT, PTE_R, PTE_X, 0};
    size_t initial = nr_free_pages();
    struct Page *a = alloc_page();
    assert(a);
    for (unsigned i = 0; i < sizeof(permissions) / sizeof(permissions[0]); i++) {
        assert(page_insert(boot_pgdir_va, a, 0x4100, permissions[i]) == 0);
        pte_t *p = get_pte(boot_pgdir_va, 0x4000, 0);
        assert((*p & 0x3ff) == (permissions[i] | PTE_V));
        assert(page_ref(a) == 1 && nr_free_pages() == initial - 3);
    }
    page_remove(boot_pgdir_va, 0x4000);
    assert(page_ref(a) == 0 && nr_free_pages() == initial - 2);
    release_empty_branch(0);
    assert(nr_free_pages() == initial);
    cprintf("PASS permission matrix: replace RW/U/software flags with R, X and zero; same ref/page count\n");
}

static void index_boundaries(void) {
    stage = "index boundaries";
    const uintptr_t addresses[2][2] = {{0x1ff000, 0x200000}, {0x3ffff000, 0x40000000}};
    for (unsigned test = 0; test < 2; test++) {
        size_t initial = nr_free_pages();
        struct Page *a = alloc_page(), *b = alloc_page();
        assert(a && b);
        uintptr_t x = addresses[test][0], y = addresses[test][1];
        assert(page_insert(boot_pgdir_va, a, x, PTE_R) == 0);
        assert(page_insert(boot_pgdir_va, b, y, PTE_R) == 0);
        assert(get_page(boot_pgdir_va, x + 0x100, NULL) == a);
        assert(get_page(boot_pgdir_va, y + 0x100, NULL) == b);
        assert(page_ref(a) == 1 && page_ref(b) == 1);
        unsigned table_pages = test == 0 ? 3 : 4;
        assert(nr_free_pages() == initial - 2 - table_pages);
        assert(page_ref(pde2page(boot_pgdir_va[0])) == 1);
        if (test) assert(page_ref(pde2page(boot_pgdir_va[1])) == 1);
        page_remove(boot_pgdir_va, x); page_remove(boot_pgdir_va, y);
        assert(nr_free_pages() == initial - table_pages);
        release_empty_branch(0); release_empty_branch(1);
        assert(nr_free_pages() == initial);
        cprintf("PASS index boundary: %s, table_pages=%u, lookups/refs/page recovery\n",
                test == 0 ? "2 MiB" : "1 GiB", table_pages);
    }
}

static void prebuild_legal_path(void) {
    /* Fixture-owned path exists before the tested operation. No generated
     * entries are patched; this does not validate production table creation. */
    struct Page *middle = alloc_page(), *leaf = alloc_page();
    assert(middle && leaf && boot_pgdir_va[0] == 0);
    memset(page2kva(middle), 0, PGSIZE); memset(page2kva(leaf), 0, PGSIZE);
    set_page_ref(middle, 1); set_page_ref(leaf, 1);
    boot_pgdir_va[0] = ptd_create(page2ppn(middle));
    ((pte_t *)page2kva(middle))[0] = ptd_create(page2ppn(leaf));
    flush_tlb();
}

static void arm_fault(uintptr_t cause, uintptr_t epc, uintptr_t resume) {
    expected_cause = cause;
    expected_address = 0x4000;
    expected_epc = epc;
    expected_resume = resume;
    observed_faults = 0;
    expected_fault_armed = 1;
}

static void protect_fault(void) {
    stage = "warmed writable mapping becomes readonly";
    size_t initial = nr_free_pages();
    prebuild_legal_path();
    struct Page *a = alloc_page();
    assert(a);
    const uintptr_t original = 0x1111222233334444ULL;
    assert(page_insert(boot_pgdir_va, a, 0x4000, PTE_R | PTE_W | PTE_A | PTE_D) == 0);
    mapping_probe_store(0x4000, original); /* Real successful write, warms access. */
    assert(*(volatile uintptr_t *)page2kva(a) == original);
    size_t allocated = nr_free_pages();
    assert(page_insert(boot_pgdir_va, a, 0x4000, PTE_R | PTE_A | PTE_D) == 0);
    assert(page_ref(a) == 1 && nr_free_pages() == allocated);
    assert(!(*get_pte(boot_pgdir_va, 0x4000, 0) & PTE_W));
    assert(mapping_probe_load(0x4000) == original); /* R is still allowed. */
    arm_fault(15, (uintptr_t)mapping_store_fault, (uintptr_t)mapping_store_resume);
    mapping_probe_store(0x4000, 0x5555666677778888ULL);
    assert(observed_faults == 1 && !expected_fault_armed);
    assert(*(volatile uintptr_t *)page2kva(a) == original && page_ref(a) == 1);
    page_remove(boot_pgdir_va, 0x4000); release_empty_branch(0);
    assert(nr_free_pages() == initial);
    cprintf("PASS protection: real write -> R-only -> exact store page fault; data/ref/count preserved\n");
}

static void unmap_fault(void) {
    stage = "deleted mapping with physical page reuse";
    size_t initial = nr_free_pages();
    prebuild_legal_path();
    struct Page *a = alloc_page();
    assert(a);
    *(volatile uintptr_t *)page2kva(a) = 0x1111222233334444ULL;
    assert(page_insert(boot_pgdir_va, a, 0x4000, PTE_R | PTE_W | PTE_A | PTE_D) == 0);
    assert(mapping_probe_load(0x4000) == 0x1111222233334444ULL);
    page_remove(boot_pgdir_va, 0x4000);
    assert(page_ref(a) == 0 && *get_pte(boot_pgdir_va, 0x4000, 0) == 0);
    struct Page *reused = alloc_page();
    assert(reused == a);
    const uintptr_t fresh = 0x5555666677778888ULL;
    *(volatile uintptr_t *)page2kva(reused) = fresh;
    arm_fault(13, (uintptr_t)mapping_load_fault, (uintptr_t)mapping_load_resume);
    (void)mapping_probe_load(0x4000);
    assert(observed_faults == 1 && !expected_fault_armed);
    assert(*(volatile uintptr_t *)page2kva(reused) == fresh && page_ref(reused) == 0);
    size_t before_repeat = nr_free_pages();
    page_remove(boot_pgdir_va, 0x4000);
    assert(nr_free_pages() == before_repeat);
    free_page(reused); release_empty_branch(0);
    assert(nr_free_pages() == initial);
    cprintf("PASS deletion: warmed read -> unmap -> same physical page reused -> exact load page fault\n");
}

static void hardware_alias(bool prebuilt) {
    stage = prebuilt ? "prebuilt legal path" : "production-generated path";
    size_t initial = nr_free_pages();
    if (prebuilt) {
        /* Caller-provided valid path, not a repair of get_pte-generated flags. */
        struct Page *middle = alloc_page(), *leaf = alloc_page();
        assert(middle && leaf && boot_pgdir_va[0] == 0);
        memset(page2kva(middle), 0, PGSIZE);
        memset(page2kva(leaf), 0, PGSIZE);
        set_page_ref(middle, 1); set_page_ref(leaf, 1);
        boot_pgdir_va[0] = ptd_create(page2ppn(middle));
        ((pte_t *)page2kva(middle))[0] = ptd_create(page2ppn(leaf));
        flush_tlb();
    }
    struct Page *a = alloc_page(), *b = alloc_page();
    assert(a && b);
    *(volatile uintptr_t *)page2kva(a) = 0x123456789abcdef0ULL;
    *(volatile uintptr_t *)page2kva(b) = 0xfedcba9876543210ULL;
    volatile uintptr_t *x = (volatile uintptr_t *)0x4000;
    volatile uintptr_t *y = (volatile uintptr_t *)0x5000;
    uint32_t perm = PTE_R | PTE_W | PTE_A | PTE_D;
    assert(page_insert(boot_pgdir_va, a, (uintptr_t)x, perm) == 0);
    assert(*x == 0x123456789abcdef0ULL); /* Warm the old translation. */
    assert(page_insert(boot_pgdir_va, a, (uintptr_t)y, perm) == 0);
    *y = 0x1111222233334444ULL;
    assert(*x == 0x1111222233334444ULL);
    assert(page_insert(boot_pgdir_va, b, (uintptr_t)x, perm) == 0);
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
    if (CHECK_MODE == 0) {
        check_alloc_page();
        stage = "unchanged check_pgdir"; check_pgdir();
        stage = "unchanged tools PMM test"; test_lab4_pmm_basic();
        release_empty_branch(0);
        cprintf("PASS unchanged framework software checks\n");
        software_matrix();
        allocation_failure(0, 0, 0);
        allocation_failure(1, 0, 0);
        allocation_failure(0, 1, 0);
        allocation_failure(1, 0, 1);
        permission_matrix();
        index_boundaries();
    } else if (CHECK_MODE == 1) {
        hardware_alias(1);
    } else if (CHECK_MODE == 2) {
        hardware_alias(0);
    } else if (CHECK_MODE == 3) {
        stage = "unchanged check_boot_pgdir"; check_boot_pgdir();
    } else if (CHECK_MODE == 4) {
        protect_fault();
    } else {
        unmap_fault();
    }
    cprintf("MAPPING_PASS mode=%d: bounded independent check, not complete Lab4\n", CHECK_MODE);
    sbi_shutdown();
    while (1) asm volatile("");
}
