python
import gdb, pathlib, time
_checkpoint_dir = pathlib.Path('/home/lhl/os2026/lab4/report/evidence/2026-11-10-demo/mapping')
def capture_checkpoint(name):
    gdb.write("CAPTURE_CHECKPOINT " + name + "\n")
    gdb.flush()
    (_checkpoint_dir / "request.txt").write_text(name)
    deadline = time.monotonic() + 25
    while not (_checkpoint_dir / ("ack-" + name)).exists():
        if time.monotonic() > deadline:
            raise RuntimeError("Live frontend did not acknowledge " + name)
        time.sleep(0.03)
end
set pagination off
set confirm off
set style enabled off
set print pretty off
file /home/lhl/os2026/lab4/report/evidence/2026-11-10-demo/software-image/kernel
set architecture riscv:rv64
target remote 127.0.0.1:55267
set trace-commands on
hbreak page_insert
condition 1 la == 0x4100
continue
set $dbg_a = page
set $dbg_root = pgdir
set $dbg_va = la
set $dbg_free = free_area.nr_free
printf "BEFORE first mapping\n"
p/x $pc
p/x boot_pgdir_pa
p/x (($satp & 0xfffffffffff) << 12)
p/x $dbg_root[0]
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER first mapping: root -> L1 -> L0 -> data PTE\n"
set $dbg_l2 = $dbg_root[($dbg_va >> 30) & 511]
set $dbg_l1 = (pte_t *)((($dbg_l2 >> 10) << 12) + va_pa_offset)
set $dbg_mid = $dbg_l1[($dbg_va >> 21) & 511]
set $dbg_l0 = (pte_t *)((($dbg_mid >> 10) << 12) + va_pa_offset)
set $dbg_xslot = $dbg_l0 + (($dbg_va >> 12) & 511)
p/x $dbg_l2
p/x $dbg_mid
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free")) - 2
python capture_checkpoint("A02-mapping-created")
condition 1 la == 0x4000 && page == $dbg_a && perm == 0
continue
printf "BEFORE same-page remap\n"
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER same-page remap: flags replaced, no extra reference\n"
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free")) - 2
python capture_checkpoint("A03-same-page-remap")
condition 1 la == 0x4000 && page != $dbg_a && perm == 2
continue
set $dbg_b = page
printf "BEFORE replacing X: A also mapped at Y\n"
p $dbg_a->ref
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 2
finish
printf "AFTER replacing X: A remains referenced by Y\n"
p/x *$dbg_xslot
p $dbg_a->ref
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("$dbg_b->ref")) == 1
python capture_checkpoint("A04-shared-page-replaced")
disable 1
hbreak page_remove
condition 2 la == 0x5100
continue
set $dbg_yslot = $dbg_l0 + 5
printf "BEFORE removing Y (last reference to A)\n"
p/x *$dbg_yslot
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER removing Y\n"
p/x *$dbg_yslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 0
condition 2 la == 0x4000
continue
printf "BEFORE removing X (last reference to B)\n"
p $dbg_b->ref
finish
printf "AFTER removing X: data freed, two table pages remain\n"
p/x *$dbg_xslot
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_b->ref")) == 0
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free"))
python capture_checkpoint("A05-unmap-and-free")
delete breakpoints
detach
quit
