/* Separate test kernel: the normal kern/init/init.c is never overwritten. */
#include <stdio.h>
#include <string.h>

int kern_init(void) __attribute__((noreturn));

int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    const char *expected =
        "FMT: ok -42 42 2a 0x80200000 Z % 0000002a "
        "-9223372036854775808 18446744073709551615\n";
    int count = cprintf("FMT: %s %d %u %x %p %c %% %08x %lld %llu\n",
                        "ok", -42, 42U, 42U, (void *)0x80200000UL,
                        'Z', 42U, (-9223372036854775807LL - 1),
                        18446744073709551615ULL);
    if (count == strlen(expected)) {
        cputs("LAB1_CONSOLE_CHECK_PASS");
    } else {
        cprintf("LAB1_CONSOLE_CHECK_FAIL: count=%d\n", count);
    }
    while (1)
        ;
}
