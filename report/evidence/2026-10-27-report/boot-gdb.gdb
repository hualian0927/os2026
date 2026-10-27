set pagination off
set confirm off
set architecture riscv:rv64
set remotetimeout 5
file /tmp/os2026-lab3-report-qj74bh6h/foundation/kernel
target remote 127.0.0.1:47795
hbreak *0x80200000
continue
info registers pc a0 a1 satp
x/4bx $a1
hbreak *kern_init
continue
info registers pc sp satp
p/x &bootstacktop
x/gx ((char *)&boot_page_table_sv39)+2*8
x/gx ((char *)&boot_page_table_sv39)+511*8
hbreak pmm_init
continue
finish
p/x npage
p/x nbase
p/x pages
x/s pmm_manager->name
detach
quit
