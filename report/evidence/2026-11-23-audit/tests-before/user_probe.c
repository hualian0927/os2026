/* Runs in real U mode, linked with ACTUAL user/libs/syscall.c. */
#include <defs.h>
#include <syscall.h>
#include <error.h>
#define CHECK(x) do { if (!(x)) { sys_exit(0xbad); while (1) {} } } while (0)

void user_start(void) {
    volatile uint64_t guard[4] = {0x123456789abcdef0ULL, 0xfedcba9876543210ULL,
                                 0x5555555555555555ULL, 0xaaaaaaaaaaaaaaaaULL};
    volatile uint64_t *data = (volatile uint64_t *)0x402000;
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
