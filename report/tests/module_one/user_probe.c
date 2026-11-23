/* Runs in real U mode, linked with ACTUAL user/libs/syscall.c. */
#include <defs.h>
#include <syscall.h>
#include <error.h>
#include <unistd.h>
#define CHECK(x) do { if (!(x)) { sys_exit(0xbad); while (1) {} } } while (0)
extern int64_t raw_ecall(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5);

void user_start(void) {
    volatile uint64_t guard[4] = {0x123456789abcdef0ULL, 0xfedcba9876543210ULL,
                                 0x5555555555555555ULL, 0xaaaaaaaaaaaaaaaaULL};
    volatile uint64_t *data = (volatile uint64_t *)0x402000;
    /* Real U-mode rejection paths, separately from software trapframe tests.
     * Full 64-bit negative results also verify sign extension in frame a0. */
    CHECK(raw_ecall(SYS_getpid, 0, 0, 0, 0, 0) == 2412351);
    uint64_t invalid[] = {0, SYS_clone, 32, 0x100000001ULL, (uint64_t)-1};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); i++) {
        CHECK(raw_ecall(invalid[i], 0, 0, 0, 0, 0) == -E_INVAL);
    }
    /* These must be rejected BEFORE a do_* probe is reached. */
    CHECK(raw_ecall(SYS_wait, 23, 0xffffffffc0200000ULL, 0, 0, 0) == -E_INVAL);
    CHECK(raw_ecall(SYS_wait, 23, 0x400000, 0, 0, 0) == -E_INVAL); /* RX */
    CHECK(raw_ecall(SYS_wait, 23, 0x200000, 0, 0, 0) == -E_INVAL); /* no VMA */
    CHECK(raw_ecall(SYS_wait, 23, 0x403ffd, 0, 0, 0) == -E_INVAL); /* four bytes cross end */
    CHECK(raw_ecall(SYS_wait, 23, (uint64_t)-2, 0, 0, 0) == -E_INVAL);
    CHECK(raw_ecall(SYS_kill, 0x100000001ULL, 0, 0, 0, 0) == -E_INVAL);
    CHECK(raw_ecall(SYS_exec, 0, 0, 0xffffffffc0200000ULL, 16, 0) == -E_INVAL);
    CHECK(raw_ecall(SYS_exec, 0, 4, 0x402000, 16, 0) == -E_INVAL);
    for (unsigned i = 0; i < 16; i++) {
        CHECK(sys_getpid() == 2412351);
        CHECK(sys_fork() == -E_NO_MEM);
        *data = 0xaabbccdd00000000ULL;
        CHECK(sys_wait(23, (int *)data) == -E_BAD_PROC);
        CHECK(*data == 0xaabbccdd11223344ULL);
        CHECK(sys_wait(-1, NULL) == -E_BAD_PROC);
        CHECK(sys_kill(-1) == -E_INVAL);
        CHECK(sys_yield() == -E_BUSY);
        CHECK(sys_putc('^') == 0);
        CHECK(sys_pgdir() == 0);
        CHECK(sys_exit(-7) == -E_KILLED); /* Deliberately returning test probe. */
        CHECK(guard[0] == 0x123456789abcdef0ULL && guard[1] == 0xfedcba9876543210ULL);
        CHECK(guard[2] == 0x5555555555555555ULL && guard[3] == 0xaaaaaaaaaaaaaaaaULL);
    }
    sys_exit(0x600d); /* Test probe edits the real frame to return to S fixture. */
    while (1) {}
}
