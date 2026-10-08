#include <stdio.h>
#include <string.h>
int kern_init(void) __attribute__((noreturn));

int kern_init(void) {
    extern char edata[], end[];
    /* No C runtime is present: clear the linker-defined BSS ourselves. */
    memset(edata, 0, end - edata);

    const char *message = "(THU.CST) os is loading ...\n";
    cprintf("%s\n\n", message);
    /* The minimal kernel has no scheduler yet and must not return. */
    while (1)
        ;
}
