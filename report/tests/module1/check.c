/* Independent module-one fixture. Not a production exception/timer handler. */
#include <assert.h>
#include <defs.h>
#include <riscv.h>
#include <sbi.h>
#include <stdio.h>
#include <string.h>
#include <trap.h>

#define ROUNDS 64
volatile uintptr_t observed[32];
volatile uintptr_t expected_sp, expected_gp, expected_tp;
volatile uintptr_t wrapper_sp_before, wrapper_sp_after;
static volatile unsigned round_id, trap_count, helper_calls;
static uintptr_t original_stval;
extern uintptr_t abi_checked_probe(void);
extern void probe_breakpoint(void), probe_resume(void), __alltraps(void);

_Static_assert(sizeof(struct trapframe) == 288, "RV64 frame size");
_Static_assert(__builtin_offsetof(struct trapframe, gpr.sp) == 16, "SP slot");
_Static_assert(__builtin_offsetof(struct trapframe, status) == 256, "status slot");
_Static_assert(__builtin_offsetof(struct trapframe, epc) == 264, "EPC slot");
_Static_assert(__builtin_offsetof(struct trapframe, badvaddr) == 272, "stval slot");
_Static_assert(__builtin_offsetof(struct trapframe, cause) == 280, "cause slot");

void __noreturn __panic(const char *file, int line, const char *fmt, ...) {
    clear_csr(sstatus, SSTATUS_SIE);
    cprintf("MODULE_ONE_FAIL round=%u trap_count=%u at %s:%d: ",
            round_id, trap_count, file, line);
    va_list ap;
    va_start(ap, fmt);
    vcprintf(fmt, ap);
    va_end(ap);
    cprintf("\n");
    sbi_shutdown();
    while (1) asm volatile("");
}

static uintptr_t entry_register(unsigned i) {
    if (i == 0) return 0;
    if (i == 2) return expected_sp;
    if (i == 3) return expected_gp;
    if (i == 4) return expected_tp;
    return 0x100 + i;
}

/* A real noinline C call with stack storage, loop, return value and side effect. */
static uintptr_t __attribute__((noinline)) c_work(uintptr_t seed) {
    volatile uintptr_t values[8];
    uintptr_t result = 0;
    for (unsigned i = 0; i < 8; i++) {
        values[i] = seed + i;
        result += values[i];
    }
    helper_calls++;
    return result;
}

static void __attribute__((noinline)) inspect_frame(struct trapframe *tf) {
    uintptr_t saved[32];
    memcpy(saved, &tf->gpr, sizeof(saved));
    assert(trap_count == round_id); /* Also catches replay after a lost EPC edit. */
    assert((uintptr_t)tf == expected_sp - sizeof(*tf));
    assert(((uintptr_t)tf & 15) == 0);
    assert(tf->cause == 3 && tf->epc == (uintptr_t)probe_breakpoint);
    assert((tf->status & SSTATUS_SPP) != 0);
    assert((tf->status & SSTATUS_SIE) == 0);
    assert(((tf->status & SSTATUS_SPIE) != 0) == (round_id & 1));
    assert(read_csr(sscratch) == 0);
    for (unsigned i = 0; i < 32; i++) assert(saved[i] == entry_register(i));
    assert(c_work(round_id + 3) == 8 * (round_id + 3) + 28);

    /* The selected successor skips a 32-bit intentionally illegal instruction. */
    tf->epc = (uintptr_t)probe_resume;
    if (round_id & 2) {
        tf->gpr.a0 = 0xabc;
        tf->gpr.s0 = 0xdef;
        tf->gpr.s1 = 0x123;
        tf->gpr.t0 = 0x456;
    }
    if (round_id & 4) tf->status ^= SSTATUS_SPIE;
    original_stval = read_csr(stval);
    tf->cause = 0x123;
    tf->badvaddr = 0x456;
    trap_count++;
}

/* Only this fixture is linked, never the incomplete formal trap.c. */
void trap(struct trapframe *tf) {
    inspect_frame(tf);
    asm volatile("li t0, 1; li t1, 2; li t2, 3; li a0, 4; li a1, 5; "
                 "li a2, 6; li a3, 7; li a4, 8; li a5, 9; li a6, 10; "
                 "li a7, 11; li t3, 12; li t4, 13; li t5, 14; li t6, 15"
        ::: "t0", "t1", "t2", "a0", "a1", "a2", "a3", "a4", "a5",
            "a6", "a7", "t3", "t4", "t5", "t6");
}

static inline uintptr_t current_sp(void) {
    uintptr_t value;
    asm volatile("mv %0, sp" : "=r"(value));
    return value;
}

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    clear_csr(sstatus, SSTATUS_SIE);
    /* No asynchronous source is enabled, even in SIE=1 test combinations. */
    write_csr(sie, 0);
    write_csr(sscratch, 0);
    write_csr(stvec, (uintptr_t)__alltraps);
    uintptr_t first_probe_sp = 0;
    for (round_id = 0; round_id < ROUNDS; round_id++) {
        memset((void *)observed, 0xff, sizeof(observed));
        uintptr_t before = current_sp();
        if (round_id & 1) set_csr(sstatus, SSTATUS_SIE);
        else clear_csr(sstatus, SSTATUS_SIE);
        uintptr_t abi_failure = abi_checked_probe();
        uintptr_t returned_status = read_csr(sstatus);
        clear_csr(sstatus, SSTATUS_SIE);
        uintptr_t after = current_sp();
        assert(abi_failure == 0);
        assert(before == after);
        assert(wrapper_sp_before == wrapper_sp_after);
        if (round_id == 0) first_probe_sp = expected_sp;
        assert(expected_sp == first_probe_sp);
        for (unsigned i = 0; i < 32; i++) {
            uintptr_t expected = entry_register(i);
            if (round_id & 2) {
                if (i == 10) expected = 0xabc;
                if (i == 8) expected = 0xdef;
                if (i == 9) expected = 0x123;
                if (i == 5) expected = 0x456;
            }
            assert(observed[i] == expected);
        }
        assert(((returned_status & SSTATUS_SIE) != 0) ==
               (((round_id & 1) != 0) ^ ((round_id & 4) != 0)));
        assert((returned_status & SSTATUS_SPIE) != 0);
        assert((returned_status & SSTATUS_SPP) == 0);
        assert(read_csr(sscratch) == 0);
        assert(read_csr(scause) == 3 && read_csr(stval) == original_stval);
        assert(trap_count == round_id + 1 && helper_calls == round_id + 1);
    }
    cprintf("PASS frame: actual tf, 288-byte layout, original SP, 16-byte alignment\n");
    cprintf("PASS registers: entry/return slots, RA, gp/tp preserved, real C calls/clobbers\n");
    cprintf("PASS edits: selected EPC successor, a0/s0/s1/t0 frame changes\n");
    cprintf("PASS stack: 64 successive traps, caller/probe SP stable, driver callee-saved ABI\n");
    cprintf("PASS state: S-mode SPP, SIE/SPIE transitions, sscratch, diagnostic fields\n");
    cprintf("MODULE_ONE_PASS: 64 real traps; all 8 state/edit combinations repeated 8 times\n");
    sbi_shutdown();
    while (1) asm volatile("");
}
