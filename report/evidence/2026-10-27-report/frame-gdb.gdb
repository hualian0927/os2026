set pagination off
set confirm off
set architecture riscv:rv64
set remotetimeout 5
file /home/lhl/os2026-lab3/report/tests/module1/results/run-saojv__q/O0/kernel
target remote 127.0.0.1:36695
hbreak inspect_frame if round_id == 2
continue
set $frame_a = tf
p round_id
p sizeof(*$frame_a)
p/x $frame_a
p/x expected_sp
p ((uintptr_t)$frame_a + sizeof(*$frame_a)) == expected_sp
p/x $frame_a->gpr.sp
p/x $frame_a->gpr.ra
p/x $frame_a->epc
p/x $frame_a->status
thbreak *probe_resume
continue
info registers pc sp ra a0 s0 s1 t0
p/x ($sstatus & 0x2)
p/x (($sstatus >> 5) & 1)
p/x (($sstatus >> 8) & 1)
detach
quit
