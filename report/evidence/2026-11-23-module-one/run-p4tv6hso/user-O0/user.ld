ENTRY(user_start)
SECTIONS { . = 0x400000; .text : { *probe.o(.text*) *(.text*) } .rodata : { *(.rodata*) } .data : { *(.sdata*) *(.data*) } .bss : { *(.sbss*) *(.bss*) } }
