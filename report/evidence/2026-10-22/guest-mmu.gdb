set pagination off
set trace-commands on
set architecture riscv:rv64
target remote localhost:15342
printf "\n=== RESET_STATE ===\n"
info registers pc satp
hbreak *0x80200000
continue
printf "\n=== BEFORE_PAGING ===\n"
info registers pc satp
hbreak *kern_init
continue
printf "\n=== AFTER_PAGING ===\n"
info registers pc satp
p/x &boot_page_table_sv39
x/gx ((char *)&boot_page_table_sv39)+2*8
x/gx ((char *)&boot_page_table_sv39)+511*8
printf "\n=== VIRTUAL_AND_IDENTITY_ALIASES ===\n"
x/4i kern_init
x/4i ((unsigned long)kern_init-0xffffffff40000000)
set $leaf=*(unsigned long *)(((char *)&boot_page_table_sv39)+511*8)
p/x (($leaf>>10)<<12)+((unsigned long)kern_init&0x3fffffff)
printf "\n=== GUEST_TRACE_COMPLETE ===\n"
detach
quit
