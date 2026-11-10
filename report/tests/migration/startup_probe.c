/* Independent startup observer: real entry/DTB/console, no Lab4 modules. */
#include <defs.h>
#include <string.h>
#include <stdio.h>
#include <console.h>
#include <dtb.h>
#include <sbi.h>

int kern_init(void) __attribute__((noreturn));
int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);
    cons_init();
    dtb_init();
    cprintf("STARTUP_PROBE_PASS: hart=%lu dtb=%lx base=%lx size=%lx\n",
            (unsigned long)boot_hartid, (unsigned long)boot_dtb,
            (unsigned long)get_memory_base(), (unsigned long)get_memory_size());
    sbi_shutdown();
    while (1) asm volatile("");
}
