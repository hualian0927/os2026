/* Isolated forwarding probes, NOT implementations of the process backends.
 * Include the actual dispatcher to inspect its five-argument internal ABI. */
#include <syscall.c>
#include <console.h>
#include <riscv.h>
#include <sbi.h>
#include <string.h>

static struct proc_struct process;
struct proc_struct *current = &process;
static struct trapframe frame, outer;
static struct mm_struct mm;
static struct vma_struct regions[3];
static unsigned hits, pgdir_hits;
static int backend_result, observed_int;
static uintptr_t observed_ptr, expected_epc;
static bool edit_exec;
static uint64_t observed_args[5];
extern void test_lab5_syscall(void);

void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    clear_csr(sstatus, SSTATUS_SIE);
    cprintf("INTERFACE_FAIL %s:%d ", file, line);
    va_list ap; va_start(ap, fmt); vcprintf(fmt, ap); va_end(ap);
    cprintf("\n"); sbi_shutdown(); while (1) {}
}
void schedule(void) { panic("Unexpected scheduling in interface fixture"); }
static void observed(void) {
    assert(current->tf == &frame && frame.epc == 0x1004);
    hits++;
}
int do_exit(int code) { observed(); observed_int = code; return backend_result; }
int do_fork(uint32_t flags, uintptr_t stack, struct trapframe *tf) {
    observed(); assert(flags == 0 && tf == &frame && stack == frame.gpr.sp);
    return backend_result;
}
int do_wait(int pid, int *store) {
    observed(); observed_int = pid; observed_ptr = (uintptr_t)store;
    return backend_result; /* No wait/reap/copy is simulated. */
}
int do_execve(const char *name, size_t len, unsigned char *binary, size_t size) {
    observed();
    assert((uintptr_t)name == frame.gpr.a1 && len == frame.gpr.a2);
    assert((uintptr_t)binary == frame.gpr.a3 && size == frame.gpr.a4);
    if (edit_exec) {
        frame.epc = expected_epc = 0x456780;
        frame.gpr.sp = 0x712340;
        frame.gpr.s2 = 0xfedcba9876543210ULL;
        frame.status ^= SSTATUS_SPIE;
    }
    return backend_result;
}
int do_yield(void) { observed(); return backend_result; }
int do_kill(int pid) { observed(); observed_int = pid; return backend_result; }
void print_pgdir(void) { pgdir_hits++; } /* Call-observation probe only. */

static int dispatch(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5) {
    memset(&frame, 0, sizeof(frame));
    frame.cause = CAUSE_USER_ECALL;
    frame.epc = 0x1000; expected_epc = 0x1004;
    frame.gpr.sp = 0x778800;
    frame.gpr.a0 = num; frame.gpr.a1 = a1; frame.gpr.a2 = a2;
    frame.gpr.a3 = a3; frame.gpr.a4 = a4; frame.gpr.a5 = a5;
    frame.gpr.s2 = 0xabcdef;
    current->tf = &outer;
    trap(&frame); /* Actual trap installs/restores current->tf. */
    assert(current->tf == &outer && frame.epc == expected_epc);
    assert(frame.gpr.a1 == a1 && frame.gpr.a5 == a5);
    assert(frame.gpr.a0 == (uintptr_t)(intptr_t)(int)frame.gpr.a0);
    return (int)frame.gpr.a0;
}
static int five_args(uint64_t args[]) {
    observed(); memcpy(observed_args, args, sizeof(observed_args)); return -E_NO_MEM;
}
static void reject(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    unsigned before = hits;
    assert(dispatch(num, a1, a2, a3, a4, 0) == -E_INVAL);
    assert(hits == before);
}
static void set_regions(void) {
    memset(&mm, 0, sizeof(mm)); list_init(&mm.mmap_list);
    uintptr_t starts[3] = {USERBASE + 0x1000, USERBASE + 0x5000, USERBASE + 0x9000};
    for (unsigned i = 0; i < 3; i++) {
        regions[i].vm_mm = &mm;
        regions[i].vm_start = starts[i]; regions[i].vm_end = starts[i] + 0x2000;
        regions[i].vm_flags = i == 1 ? VM_READ : VM_READ | VM_WRITE;
        if (i == 2) regions[i].vm_flags |= VM_STACK;
        list_add_before(&mm.mmap_list, &regions[i].list_link);
    }
    current->mm = &mm;
}

int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    clear_csr(sstatus, SSTATUS_SIE); write_csr(sie, 0);
    cons_init(); idt_init(); current = &process; current->pid = 2412351;
    /* Run the supplied syscall assertions unchanged, with mm == NULL. */
    test_lab5_syscall();
    unsigned before = hits;
    uint64_t invalid[] = {0, 5, 11, 17, 19, 20, 21, 22, 32, 0xffffffff,
                          0x100000001ULL, 0xffffffffffffffffULL};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); i++)
        reject(invalid[i], 0, 0, 0, 0);
    assert(hits == before);
    cprintf("PASS: original Syscall test, holes, bounds and full-width rejection\n");

    int (*original)(uint64_t[]) = syscalls[SYS_getpid];
    syscalls[SYS_getpid] = five_args;
    uint64_t values[5] = {0x123456789abcdef0ULL, 0xfedcba9876543210ULL,
                         0x100000003ULL, 0x8000000000000004ULL, 0x500000005ULL};
    assert(dispatch(SYS_getpid, values[0], values[1], values[2], values[3], values[4]) == -E_NO_MEM);
    assert(memcmp(values, observed_args, sizeof(values)) == 0);
    syscalls[SYS_getpid] = original;
    assert(dispatch(SYS_getpid, 0, 0, 0, 0, 0) == current->pid);
    backend_result = -E_BUSY;
    assert(dispatch(SYS_exit, (uint64_t)-12LL, 0, 0, 0, 0) == -E_BUSY && observed_int == -12);
    assert(dispatch(SYS_exit, (uint64_t)-2147483648LL, 0, 0, 0, 0) == -E_BUSY);
    assert(dispatch(SYS_kill, 2147483647, 0, 0, 0, 0) == -E_BUSY);
    reject(SYS_exit, 0x100000001ULL, 0, 0, 0);
    reject(SYS_kill, 0x80000000ULL, 0, 0, 0);
    backend_result = 123;
    assert(dispatch(SYS_fork, 0, 0, 0, 0, 0) == 123);
    backend_result = -E_BAD_PROC;
    assert(dispatch(SYS_wait, (uint64_t)-1LL, 0, 0, 0, 0) == -E_BAD_PROC && observed_int == -1);
    assert(dispatch(SYS_wait, 0, 0, 0, 0, 0) == -E_BAD_PROC && observed_ptr == 0);
    backend_result = -E_KILLED;
    assert(dispatch(SYS_yield, 0, 0, 0, 0, 0) == -E_KILLED);
    assert(dispatch(SYS_putc, '!', 0, 0, 0, 0) == 0);
    reject(SYS_putc, 0x100000041ULL, 0, 0, 0);
    assert(dispatch(SYS_pgdir, 0, 0, 0, 0, 0) == 0 && pgdir_hits == 1);
    cprintf("\nPASS: all adapters, five RV64 args, signed return, fork frame and integer bounds\n");

    reject(SYS_wait, 1, KERNBASE, 0, 0); /* mm == NULL must not authorize kernel memory. */
    set_regions();
    uintptr_t rw = regions[0].vm_start, ro = regions[1].vm_start;
    assert(dispatch(SYS_wait, 1, rw, 0, 0, 0) == -E_KILLED && observed_ptr == rw);
    assert(dispatch(SYS_wait, 1, regions[0].vm_end - 4, 0, 0, 0) == -E_KILLED);
    reject(SYS_wait, 1, regions[0].vm_end - 3, 0, 0);
    reject(SYS_wait, 1, ro, 0, 0);
    reject(SYS_wait, 1, KERNBASE, 0, 0);
    reject(SYS_wait, 1, (uintptr_t)-2, 0, 0);
    reject(SYS_wait, 1, regions[2].vm_start, 0, 0);
    assert(dispatch(SYS_wait, 1, regions[2].vm_start + PGSIZE, 0, 0, 0) == -E_KILLED);
    reject(SYS_wait, 0x100000001ULL, rw, 0, 0);

    assert(dispatch(SYS_exec, ro, 4, ro + 32, 16, 0) == -E_KILLED);
    assert(dispatch(SYS_exec, 0, 0, ro, 16, 0) == -E_KILLED);
    reject(SYS_exec, ro, 4, ro, 0);
    reject(SYS_exec, ro, 4, ro, (uint64_t)-1);
    reject(SYS_exec, regions[1].vm_end - 2, 4, ro, 16);
    reject(SYS_exec, ro, 4, KERNBASE, 16);
    current->mm = NULL; reject(SYS_exec, 0, 0, KERNBASE, 16); current->mm = &mm;
    edit_exec = 1; backend_result = 0;
    assert(dispatch(SYS_exec, ro, 4, ro + 32, 16, 0) == 0);
    assert(frame.gpr.sp == 0x712340 && frame.gpr.s2 == 0xfedcba9876543210ULL);
    assert(frame.status & SSTATUS_SPIE);
    cprintf("PASS: actual VMA checks, NULL wait, 4-byte boundary, RO/gap/guard/overflow rejection\n");
    cprintf("PASS: backend EPC/SP/register/status edits survive dispatch and current->tf restoration\n");
    cprintf("INTERFACE_PASS: process/pgdir probes only; no ELF, fork, wait or exit business tested\n");
    sbi_shutdown(); while (1) {}
}
