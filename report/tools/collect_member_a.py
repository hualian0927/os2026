#!/usr/bin/env python3
"""Collect actual member-A runtime/GDB evidence without modifying code/."""
from pathlib import Path
import hashlib
import json
import os
import re
import shlex
import shutil
import socket
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
CODE = ROOT / "code"
OUT = ROOT / "report/evidence/2026-10-27-report"
ENV = os.environ.copy()
ENV["PATH"] = "/home/lhl/anaconda3/envs/os2026/bin:" + ENV["PATH"]


def hashes():
    return {str(p.relative_to(CODE)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(CODE.rglob("*")) if p.is_file()}


def run(name, command, timeout=60):
    with (OUT / name).open("w") as log:
        log.write("$ " + shlex.join(command) + "\n")
        log.flush()
        result = subprocess.run(command, cwd=ROOT, env=ENV, stdin=subprocess.DEVNULL,
                                stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
    if result.returncode:
        raise RuntimeError(f"{name}: exit {result.returncode}; inspect the raw log")
    return (OUT / name).read_text(errors="replace")


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def debug(name, image, elf, commands, temp):
    port = free_port()
    gdb_script = OUT / (name + ".gdb")
    gdb_script.write_text("\n".join([
        "set pagination off", "set confirm off", "set architecture riscv:rv64",
        "set remotetimeout 5", "file " + str(elf),
        f"target remote 127.0.0.1:{port}", *commands, "detach", "quit", ""]))
    cmd = ["qemu-system-riscv64", "-machine", "virt", "-m", "128M", "-smp", "1",
           "-nographic", "-bios", "default", "-kernel", str(image), "-monitor", "none",
           "-gdb", f"tcp:127.0.0.1:{port}", "-S"]
    with (OUT / (name + "-qemu.log")).open("w") as log:
        log.write("$ " + shlex.join(cmd) + "\n")
        log.flush()
        process = subprocess.Popen(cmd, env=ENV, stdin=subprocess.DEVNULL,
                                   stdout=log, stderr=subprocess.STDOUT)
        try:
            time.sleep(0.2)
            text = run(name + ".log", ["gdb-multiarch", "-batch", "-q", "-x", str(gdb_script)], 25)
            process.wait(timeout=10)
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=3)
    if process.returncode != 0:
        raise RuntimeError(name + ": debug fixture did not shut down normally")
    return text


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if (OUT / "collection.json").exists():
        raise RuntimeError("Evidence already exists; preserve it and choose a new dated directory")
    before = hashes()
    temp = Path(tempfile.mkdtemp(prefix="os2026-lab3-report-"))
    fixture = temp / "foundation"
    print("Runtime build directory:", temp, flush=True)
    run("foundation-build-entry.log", ["python3", str(ROOT / "report/tests/foundation/build_fixture.py"),
        str(CODE), str(fixture)])
    shutil.copy2(fixture / "build.log", OUT / "foundation-build.log")
    foundation = run("foundation-run.log", ["qemu-system-riscv64", "-machine", "virt",
        "-m", "128M", "-smp", "1", "-nographic", "-bios", "default", "-kernel",
        str(fixture / "ucore.img"), "-monitor", "none"], 20)
    assert "FOUNDATION_PASS:" in foundation and "FOUNDATION_FAIL" not in foundation
    print("PASS: actual startup, DTB, PMM, Best-Fit, reserved pages and SIE 0/1", flush=True)

    output = run("module1-run.log", ["python3", str(ROOT / "report/tests/module1/run.py"),
        "--source", str(CODE), "--negative-controls"])
    latest = Path((ROOT / "report/tests/module1/results/latest.txt").read_text().strip())
    destination = OUT / "module1"
    for p in sorted(latest.rglob("*")):
        if p.is_file() and p.suffix in {".log", ".json", ".txt", ".S"}:
            target = destination / p.relative_to(latest)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(p, target)
    assert "PASS image: O0" in output and "PASS image: O2" in output
    print("PASS: O0/O2 each 64 real traps and two rejected defective copies", flush=True)

    boot = debug("boot-gdb", fixture / "ucore.img", fixture / "kernel", [
        "hbreak *0x80200000", "continue", "info registers pc a0 a1 satp", "x/4bx $a1",
        "hbreak *kern_init", "continue", "info registers pc sp satp", "p/x &bootstacktop",
        "x/gx ((char *)&boot_page_table_sv39)+2*8",
        "x/gx ((char *)&boot_page_table_sv39)+511*8",
        "hbreak pmm_init", "continue", "finish", "p/x npage", "p/x nbase",
        "p/x pages", "x/s pmm_manager->name"], temp)
    assert "0x88000" in boot and "0x80000" in boot and "best_fit_pmm_manager" in boot
    assert boot.count("0x00000000200000cf") == 2
    print("PASS: actual boot GDB, DTB magic, stack, mappings and manager", flush=True)

    frame = debug("frame-gdb", latest / "O0/ucore.img", latest / "O0/kernel", [
        "hbreak inspect_frame if round_id == 2", "continue", "set $frame_a = tf",
        "p round_id", "p sizeof(*$frame_a)", "p/x $frame_a", "p/x expected_sp",
        "p ((uintptr_t)$frame_a + sizeof(*$frame_a)) == expected_sp",
        "p/x $frame_a->gpr.sp", "p/x $frame_a->gpr.ra", "p/x $frame_a->epc",
        "p/x $frame_a->status", "thbreak *probe_resume", "continue",
        "info registers pc sp ra a0 s0 s1 t0",
        "p/x ($sstatus & 0x2)", "p/x (($sstatus >> 5) & 1)",
        "p/x (($sstatus >> 8) & 1)"], temp)
    for reg, value in [("a0", "abc"), ("s0", "def"), ("s1", "123"), ("t0", "456")]:
        assert re.search(r"^" + reg + r"\s+0x" + value + r"\b", frame, re.M)
    assert re.search(r"\$\d+ = 288", frame) and re.search(r"\$\d+ = (true|1)\b", frame)
    print("PASS: actual frame GDB and edited EPC/a0/s0/s1/t0 returned", flush=True)
    after = hashes()
    assert before == after
    report = {"source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"],
              cwd=ROOT, text=True).strip(), "source_unchanged": before == after,
              "source_sha256": before, "temporary_build_directory": str(temp),
              "module1_original_results": str(latest),
              "scope": "Fresh foundation and module-one fixtures; no production handlers, full boot or grading",
              "result": "all checks passed"}
    (OUT / "collection.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print("PASS: code/ unchanged; raw evidence saved to", OUT, flush=True)


if __name__ == "__main__":
    main()
