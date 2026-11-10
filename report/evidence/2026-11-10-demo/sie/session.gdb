python
import gdb, pathlib, time
_checkpoint_dir = pathlib.Path('/home/lhl/os2026/lab4/report/evidence/2026-11-10-demo/sie')
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
file /home/lhl/os2026/lab4/report/evidence/2026-11-10-demo/foundation-image/kernel
set architecture riscv:rv64
target remote 127.0.0.1:57535
set trace-commands on
hbreak kern_init
continue
delete 1
python capture_checkpoint("CLEAR_QEMU")
hbreak /home/lhl/os2026/lab4/tests/migration/foundation.c:62
hbreak /home/lhl/os2026/lab4/tests/migration/foundation.c:63
hbreak /home/lhl/os2026/lab4/tests/migration/foundation.c:69
condition 2 enabled == 0
condition 3 enabled == 0
condition 4 enabled == 0
continue
printf "BEFORE local_intr_save\n"
p enabled
p/x ($sstatus & 2)
p/x $sie
p/x $stvec
python assert int(gdb.parse_and_eval("$sie")) == 0
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 0
continue
printf "AFTER save: flag preserves original state\n"
p outer
p/x ($sstatus & 2)
python assert int(gdb.parse_and_eval("outer")) == 0
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 0
continue
printf "AFTER local_intr_restore\n"
p outer
p/x ($sstatus & 2)
p/x $sie
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 0
python capture_checkpoint("A06-SIE-original-0")
condition 2 enabled == 1
condition 3 enabled == 1
condition 4 enabled == 1
continue
printf "BEFORE local_intr_save\n"
p enabled
p/x ($sstatus & 2)
p/x $sie
p/x $stvec
python assert int(gdb.parse_and_eval("$sie")) == 0
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 2
continue
printf "AFTER save: flag preserves original state\n"
p outer
p/x ($sstatus & 2)
python assert int(gdb.parse_and_eval("outer")) == 1
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 0
continue
printf "AFTER local_intr_restore\n"
p outer
p/x ($sstatus & 2)
p/x $sie
python assert int(gdb.parse_and_eval("$sstatus & 2")) == 2
python capture_checkpoint("A07-SIE-original-1")
delete breakpoints
detach
quit
