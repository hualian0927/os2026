/* Isolated C entry; actual production entry.S and DTB parser are linked. */
#include <defs.h>
#include <riscv.h>
#include <console.h>
#include <dtb.h>
#include <stdio.h>
#include <sbi.h>
#include <string.h>
int kern_init(void) {
    extern char edata[],end[];
    memset(edata,0,end-edata);
    clear_csr(sstatus,SSTATUS_SIE); write_csr(sie,0);
    cons_init();dtb_init();
    if (get_memory_base()==0x80000000 && get_memory_size()==128UL*1024*1024)
        cprintf("STARTUP_PASS hart=%lu DTB=%lx memory=%lx/%lx\n",boot_hartid,boot_dtb,get_memory_base(),get_memory_size());
    sbi_shutdown();while(1){}
}
