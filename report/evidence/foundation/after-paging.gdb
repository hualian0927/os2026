set pagination off
set confirm off
set architecture riscv:rv64
file /tmp/os2026-lab3-foundation-in91q43h/after-fixture/kernel
target remote 127.0.0.1:60231
hbreak *0x80200000
continue
info registers pc a0 a1 satp
x/4bx $a1
hbreak *kern_init
continue
info registers pc sp satp
p/x &bootstacktop
p/x boot_hartid
p/x boot_dtb
x/gx ((char *)&boot_page_table_sv39)+2*8
x/gx ((char *)&boot_page_table_sv39)+511*8
hbreak dtb_init
continue
hbreak pmm_init
continue
finish
p/x npage
p/x nbase
p/x pages
hbreak sbi_shutdown
continue
info registers sstatus sie
detach
quit
