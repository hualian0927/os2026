/* Independent U/S integration fixture. No production ELF/process backend. */
#include <pmm.c>
#include <console.h>
#include <dtb.h>
#include <vmm.h>
#include <trap.h>
#include <unistd.h>
#include <proc.h>
#include <clock.h>

#define USER_CODE 0x400000UL
#define USER_DATA 0x402000UL
#define USER_TOP  0x404000UL
static char trap_stack[8192] __attribute__((aligned(4096)));
static struct proc_struct process;
struct proc_struct *current;
static struct mm_struct mm;
static struct vma_struct regions[2];
static struct trapframe *expected_frame;
static unsigned exit_calls, fork_calls, wait_calls, kill_calls, yield_calls, pgdir_calls;
static unsigned timer_phase, timer_schedules;
uintptr_t kernel_resume_sp, final_status, final_sscratch;
extern char user_binary_begin[], user_binary_end[];
extern void enter_user_fixture(struct trapframe *), user_fixture_return(void);

void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    clear_csr(sstatus, SSTATUS_SIE);
    cprintf("USER_SYSCALL_FAIL %s:%d ", file, line);
    va_list ap; va_start(ap, fmt); vcprintf(fmt, ap); va_end(ap);
    cprintf("\n"); sbi_shutdown(); while (1) {}
}
static void select_kernel_return(void) {
    expected_frame->epc = (uintptr_t)user_fixture_return;
    expected_frame->gpr.sp = kernel_resume_sp;
    expected_frame->status |= SSTATUS_SPP | SSTATUS_SPIE;
}
void schedule(void) {
#ifdef TEST_TIMER_SPIN
    /* Observation/return selection only, NOT a scheduler implementation. */
    assert(timer_phase && ticks > 0 && current->need_resched);
    assert(current->tf == expected_frame && !(expected_frame->status & SSTATUS_SPP));
    assert(expected_frame->cause == ((uintptr_t)1 << 63 | IRQ_S_TIMER));
    timer_schedules++;
    current->need_resched = 0;
    select_kernel_return();
#else
    panic("No process scheduling in this fixture");
#endif
}
static void check_args(unsigned used) {
    struct trapframe *tf = current->tf;
    assert(tf == expected_frame && !(tf->status & SSTATUS_SPP));
    assert(tf->cause == CAUSE_USER_ECALL && read_csr(sscratch) == 0);
    assert(tf->epc >= USER_CODE && tf->epc < USER_DATA);
    assert(tf->gpr.sp >= USER_TOP-PGSIZE && tf->gpr.sp <= USER_TOP);
    uintptr_t args[5] = {tf->gpr.a1,tf->gpr.a2,tf->gpr.a3,tf->gpr.a4,tf->gpr.a5};
    for (unsigned i = used; i < 5; i++) assert(args[i] == 0);
}
int do_exit(int code) {
    check_args(1); assert(current->tf->gpr.a0 == SYS_exit);
    if (code == 0x600d) {
        assert(exit_calls == 16 && fork_calls == 16 && wait_calls == 32);
        assert(kill_calls == 16 && yield_calls == 16 && pgdir_calls == 16);
#ifdef TEST_TIMER_SPIN
        /* The U probe spins after this return. Only a real timer interrupt
         * and the production trap's reschedule branch can end that spin. */
        timer_phase = 1;
        clock_init();
#else
        select_kernel_return();
#endif
        return 0;
    }
    assert(code == -7); exit_calls++; return -E_KILLED;
}
int do_fork(uint32_t flags, uintptr_t stack, struct trapframe *tf) {
    check_args(0); assert(flags == 0 && tf == expected_frame && stack == tf->gpr.sp);
    assert(tf->gpr.a0 == SYS_fork); fork_calls++; return -E_NO_MEM;
}
int do_wait(int pid, int *store) {
    check_args(2); assert(current->tf->gpr.a0 == SYS_wait);
    if (store != NULL) {
        assert(pid == 23 && (uintptr_t)store == USER_DATA);
        int code = 0x11223344;
        assert(copy_to_user(current->mm, store, &code, sizeof(code)));
    } else assert(pid == -1);
    wait_calls++; return -E_BAD_PROC; /* Only parameter/copy probe, not wait semantics. */
}
int do_kill(int pid) {
    check_args(1); assert(pid == -1 && current->tf->gpr.a0 == SYS_kill);
    kill_calls++; return -E_INVAL;
}
int do_yield(void) {
    check_args(0); assert(current->tf->gpr.a0 == SYS_yield);
    yield_calls++; return -E_BUSY;
}
int do_execve(const char *name, size_t len, unsigned char *binary, size_t size) {
    panic("No exec in the U fixture; tested by software forwarding fixture");
}
void print_pgdir(void) { check_args(0); pgdir_calls++; } /* Observation only. */

int kern_init(void) {
    extern char edata[], end[], boot_page_table_sv39[];
    memset(edata, 0, end-edata); clear_csr(sstatus, SSTATUS_SIE); write_csr(sie, 0);
    cons_init(); idt_init(); dtb_init(); init_pmm_manager(); page_init();
    current = &process; current->pid = 2412351;
    memset(&mm, 0, sizeof(mm)); list_init(&mm.mmap_list); current->mm = &mm;
    for (unsigned i = 0; i < 2; i++) {
        regions[i].vm_start = i ? USER_DATA : USER_CODE;
        regions[i].vm_end = i ? USER_TOP : USER_DATA;
        regions[i].vm_flags = i ? VM_READ | VM_WRITE : VM_READ | VM_EXEC;
        regions[i].vm_mm = &mm;
        list_add_before(&mm.mmap_list, &regions[i].list_link);
    }
    size_t before = nr_free_pages();
    struct Page *allocated[6];
    for (unsigned i = 0; i < 6; i++) {
        allocated[i] = alloc_page(); assert(allocated[i]);
        memset(page2kva(allocated[i]), 0, PGSIZE);
    }
    pde_t *root = (pde_t *)boot_page_table_sv39, *mid = page2kva(allocated[0]);
    pte_t *pt = page2kva(allocated[1]);
    size_t size = user_binary_end - user_binary_begin;
    assert(size > 0 && size <= 2 * PGSIZE);
    for (unsigned i = 0; i < 2; i++) {
        size_t offset = i * PGSIZE;
        if (offset < size) memcpy(page2kva(allocated[2+i]), user_binary_begin+offset,
                                  size-offset > PGSIZE ? PGSIZE : size-offset);
        pt[PTX(USER_CODE)+i] = pte_create(page2ppn(allocated[2+i]), PTE_R|PTE_X|PTE_U|PTE_A);
    }
    pt[PTX(USER_DATA)] = pte_create(page2ppn(allocated[4]), PTE_R|PTE_W|PTE_U|PTE_A|PTE_D);
    pt[PTX(USER_TOP-PGSIZE)] = pte_create(page2ppn(allocated[5]), PTE_R|PTE_W|PTE_U|PTE_A|PTE_D);
    /* Explicit, legal TEST page tables, not production get_pte/ELF loading. */
    root[PDX1(USER_CODE)] = ptd_create(page2ppn(allocated[0]));
    mid[PDX0(USER_CODE)] = ptd_create(page2ppn(allocated[1]));
    flush_tlb(); asm volatile("fence.i" ::: "memory");
    expected_frame = (struct trapframe *)(trap_stack + sizeof(trap_stack)) - 1;
    expected_frame->status = (read_csr(sstatus)&~(SSTATUS_SPP|SSTATUS_SIE))|SSTATUS_SPIE;
    expected_frame->epc = USER_CODE; expected_frame->gpr.sp = USER_TOP;
    asm volatile("mv %0, gp" : "=r"(expected_frame->gpr.gp));
    asm volatile("mv %0, tp" : "=r"(expected_frame->gpr.tp));
    current->tf = expected_frame;
    enter_user_fixture(expected_frame); /* Existing ABI-preserving test entry. */
    assert(final_sscratch == 0 && (final_status&SSTATUS_SIE));
    root[PDX1(USER_CODE)] = 0; flush_tlb();
    for (unsigned i = 0; i < 6; i++) free_page(allocated[i]);
    assert(nr_free_pages() == before);
#ifdef TEST_TIMER_SPIN
    assert(timer_phase && timer_schedules == 1 && ticks > 0);
    cprintf("\nUSER_TIMER_PASS: real U spin interrupted at tick %lu; production handler requested scheduling; observer selected S return\n", ticks);
#else
    cprintf("\nUSER_SYSCALL_PASS: real U rejection paths + 16 wrapper rounds; actual ecall/trap/syscall/sret; backend probes, NOT processes\n");
#endif
    sbi_shutdown(); while (1) {}
}
