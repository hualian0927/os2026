#!/usr/bin/env python3
"""Local checks for lab1; no third-party Python modules or official score."""
import argparse
from pathlib import Path
import shlex
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]
EXPECTED = ("FMT: ok -42 42 2a 0x80200000 Z % 0000002a "
            "-9223372036854775808 18446744073709551615")


def run(command, log):
    result = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=40)
    with log.open("a") as output:
        output.write("$ " + shlex.join(map(str, command)) + "\n" + result.stdout)
    if result.returncode:
        raise RuntimeError(f"command failed; see {log}:\n{result.stdout}")
    return result.stdout


def wait_until(predicate, process, seconds=10):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError(f"QEMU exited early ({process.returncode})")
        if predicate():
            return
        time.sleep(0.05)
    raise RuntimeError("QEMU check timed out")


def boot(args, image, log, marker, socket_path=None):
    command = shlex.split(args.qemu) + [
        "-machine", "virt", "-nographic", "-bios", "default", "-kernel", str(image)]
    if socket_path:
        command += ["-S", "-gdb", f"unix:{socket_path},server=on,wait=off"]
    with log.open("w") as output:
        process = subprocess.Popen(command, cwd=ROOT, stdin=subprocess.DEVNULL,
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            if socket_path:
                wait_until(socket_path.exists, process)
                trace = run(shlex.split(args.gdb) + [
                    "--batch", "-nx", "-ex", "set architecture riscv:rv64",
                    "-ex", "file bin/kernel", "-ex", f"target remote {socket_path}",
                    "-x", "tools/boot.gdb"], log.parent / "gdb-startup.log")
                if "LAB1_BOOT_CHECK_PASS" not in trace:
                    raise RuntimeError("GDB startup assertions did not complete")
            wait_until(lambda: marker in log.read_text(errors="replace"), process)
            return log.read_text(errors="replace")
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


def console_image(args, directory, optimization, log):
    # Put entry.o LAST deliberately: the linker script must enforce entry order.
    sources = ["tests/console.c", "kern/libs/stdio.c", "kern/driver/console.c",
               "libs/printfmt.c", "libs/string.c", "libs/sbi.c", "kern/init/entry.S"]
    flags = ["-mcmodel=medany", "-std=gnu99", "-Wall", "-Werror", "-Wno-unused",
             "-fno-builtin", "-nostdinc", "-fno-stack-protector",
             "-ffunction-sections", "-fdata-sections", "-g", optimization,
             "-Ilibs", "-Ikern/driver", "-Ikern/mm"]
    objects = []
    for index, source in enumerate(sources):
        obj = directory / f"{index}.o"
        run([args.gcc_prefix + "gcc", *flags, "-c", source, "-o", str(obj)], log)
        objects.append(str(obj))
    elf, image = directory / "kernel", directory / "ucore.img"
    run([args.gcc_prefix + "ld", "-m", "elf64lriscv", "-nostdlib", "--gc-sections",
         "-T", "tools/kernel.ld", "-o", str(elf), *objects], log)
    run([args.gcc_prefix + "objcopy", "--strip-all", "-O", "binary", str(elf), str(image)], log)
    return image


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qemu", default="qemu-system-riscv64")
    parser.add_argument("--gdb", default="gdb-multiarch")
    parser.add_argument("--gcc-prefix", default="riscv64-unknown-elf-")
    args = parser.parse_args()
    logs = ROOT / "obj" / "check"
    logs.mkdir(parents=True, exist_ok=True)
    # Clear only this check's known output files, never the user's other logs.
    for name in ["gdb-startup.log", "console-build.log"]:
        (logs / name).write_text("")
    with tempfile.TemporaryDirectory(prefix="lab1-check-") as temp:
        directory = Path(temp)
        boot(args, ROOT / "bin/ucore.img", logs / "qemu.log",
             "(THU.CST) os is loading ...", directory / "gdb.sock")
        print("PASS: reset -> OpenSBI -> kernel, stack/tail, BSS range, boot output")
        for optimization in ["-O0", "-O2"]:
            build = directory / optimization[1:]
            build.mkdir()
            image = console_image(args, build, optimization, logs / "console-build.log")
            output = boot(args, image, logs / f"console{optimization}.log",
                          "LAB1_CONSOLE_CHECK_PASS")
            if EXPECTED not in output:
                raise RuntimeError(f"incorrect formatted output under {optimization}")
            print(f"PASS: {optimization} formatted output, character count, reordered entry object")
    print(f"Local checks passed; logs: {logs}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        raise SystemExit(f"FAIL: {error}")
