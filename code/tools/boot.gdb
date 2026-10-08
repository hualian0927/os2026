# Run after loading bin/kernel and connecting to a freshly paused QEMU.
# Usage inside make gdb: source tools/boot.gdb
set pagination off
set confirm off
python
import gdb

def reg(name):
    return int(gdb.parse_and_eval("$" + name))

def symbol(name):
    return int(gdb.parse_and_eval("&" + name))

def require(condition, message):
    if not condition:
        raise gdb.GdbError(message)

def step_to(address, limit=16):
    for _ in range(limit):
        if reg("pc") == address:
            return
        gdb.execute("si")
    require(reg("pc") == address, "single-step did not reach %#x" % address)

require(reg("pc") == 0x1000, "restart make debug: CPU must be at reset")
gdb.write("\n[reset ROM]\n")
gdb.execute("info registers pc")
gdb.execute("x/6i 0x1000")
# QEMU preloads the image before CPU execution; the bytes are already present.
gdb.execute("x/3i 0x80200000")
step_to(0x80000000)
gdb.write("\n[OpenSBI entry]\n")
gdb.execute("info registers pc a0 a1 a2")
require(symbol("kern_entry") == 0x80200000, "incorrect kernel entry address")
gdb.Breakpoint("*0x80200000", type=gdb.BP_HARDWARE_BREAKPOINT, temporary=True)
gdb.execute("continue")
require(reg("pc") == 0x80200000, "kernel entry breakpoint was not reached")
gdb.write("\n[kernel entry]\n")
gdb.execute("info registers pc sp ra")
gdb.execute("x/4i $pc")
old_ra = reg("ra")
step_to(symbol("kern_init"))
require(reg("sp") == symbol("bootstacktop"), "stack pointer was not initialized")
require(reg("sp") % 16 == 0, "stack is not ABI-aligned")
require(symbol("bootstacktop") - symbol("bootstack") == 8192, "wrong stack size")
require(reg("ra") == old_ra, "tail unexpectedly changed the return address")
gdb.write("\n[C entry, stack initialized]\n")
gdb.execute("info registers pc sp ra")
gdb.execute("p/x &bootstacktop")
gdb.Breakpoint("*cprintf", type=gdb.BP_HARDWARE_BREAKPOINT, temporary=True)
gdb.execute("continue")
require(reg("pc") == symbol("cprintf"), "kernel did not reach formatted output")
bss_start, bss_end = symbol("edata"), symbol("end")
require(bss_end >= bss_start, "invalid BSS range")
if bss_end > bss_start:
    data = gdb.selected_inferior().read_memory(bss_start, bss_end - bss_start)
    require(not any(bytes(data)), "BSS was not zeroed")
gdb.write("BSS range: %#x..%#x (%d bytes)\n" % (bss_start, bss_end, bss_end - bss_start))
gdb.write("LAB1_BOOT_CHECK_PASS\n")
end
detach
