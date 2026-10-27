#!/usr/bin/env python3
"""Build independent RV64 fixtures using the ACTUAL module-one assembly."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
DEFAULT_SOURCE = HERE.parents[1] / "lab3_new"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--negative-controls", action="store_true",
                        help="also reject two intentionally wrong assembly copies")
    args = parser.parse_args()
    source = args.source.resolve()
    results = HERE / "results"
    results.mkdir(exist_ok=True)
    run_dir = Path(tempfile.mkdtemp(prefix="run-", dir=results))
    transcript = []

    def say(message):
        print(message, flush=True)
        transcript.append(message)

    def hashes():
        return {str(p.relative_to(source)): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in sorted(source.rglob("*")) if p.is_file()}

    before = hashes()
    (run_dir / "production-before.json").write_text(json.dumps(before, indent=2))
    env = os.environ.copy()
    conda_bin = Path("/home/lhl/anaconda3/envs/os2026/bin")
    if conda_bin.is_dir():
        env["PATH"] = str(conda_bin) + os.pathsep + env["PATH"]
    commands = []

    def build_command(cmd, log):
        commands.append(cmd)
        log.write("$ " + shlex.join(cmd) + "\n")
        log.flush()
        subprocess.run(cmd, env=env, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=30)

    tool_info = {}
    for name in ["riscv64-unknown-elf-gcc", "riscv64-unknown-elf-ld",
                 "riscv64-unknown-elf-objcopy", "qemu-system-riscv64"]:
        path = shutil.which(name, path=env["PATH"])
        if path is None:
            raise RuntimeError("Required tool not found: " + name)
        tool_info[name] = {"path": path, "version": subprocess.check_output(
            [path, "--version"], env=env, text=True).splitlines()[0]}
    (run_dir / "tools.json").write_text(json.dumps(tool_info, indent=2))
    common = ["-mcmodel=medany", "-std=gnu99", "-Wall", "-Werror", "-Wno-unused",
              "-fno-builtin", "-nostdinc", "-fno-stack-protector",
              "-ffunction-sections", "-fdata-sections", "-g"]
    common += ["-I" + str(source / d) for d in
               ["libs", "kern/mm", "kern/driver", "kern/debug", "kern/trap"]]
    sources = [source / r for r in ["kern/init/entry.S", "kern/trap/trapentry.S",
               "kern/driver/console.c", "kern/libs/stdio.c", "libs/printfmt.c",
               "libs/string.c", "libs/sbi.c"]] + [HERE / "probe.S", HERE / "check.c"]
    baseline_objects = {}
    records = []

    def build(label, opt, mutant=None):
        out = run_dir / label
        out.mkdir()
        with (out / "build.log").open("w") as log:
            if mutant is None:
                objects = []
                for i, src in enumerate(sources):
                    obj = out / (str(i) + ".o")
                    build_command(["riscv64-unknown-elf-gcc", *common, opt,
                                   "-c", str(src), "-o", str(obj)], log)
                    objects.append(str(obj))
                baseline_objects[opt] = objects
            else:
                objects = list(baseline_objects[opt])
                obj = out / "mutant-trapentry.o"
                build_command(["riscv64-unknown-elf-gcc", *common, opt, "-c",
                               str(mutant), "-o", str(obj)], log)
                objects[1] = str(obj)
            build_command(["riscv64-unknown-elf-ld", "-m", "elf64lriscv",
                           "-nostdlib", "--gc-sections", "-T",
                           str(source / "tools/kernel.ld"), "-o", str(out / "kernel"),
                           *objects], log)
            build_command(["riscv64-unknown-elf-objcopy", "-O", "binary",
                           str(out / "kernel"), str(out / "ucore.img")], log)
            build_command(["riscv64-unknown-elf-objdump", "-d", str(out / "kernel")], log)
        return out

    def boot(out, expected_failure=False):
        cmd = ["qemu-system-riscv64", "-machine", "virt", "-m", "128M",
               "-smp", "1", "-nographic", "-bios", "default", "-kernel",
               str(out / "ucore.img"), "-monitor", "none"]
        (out / "qemu-command.txt").write_text(shlex.join(cmd) + "\n")
        timed_out = False
        with (out / "console.log").open("wb") as log:
            process = subprocess.Popen(cmd, env=env, stdin=subprocess.DEVNULL,
                                       stdout=log, stderr=subprocess.STDOUT)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                timed_out = True
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=3)
        output = (out / "console.log").read_text(errors="replace")
        passed = "MODULE_ONE_PASS:" in output
        failed = "MODULE_ONE_FAIL" in output
        # A timeout alone does not prove the intended defect was detected.
        accepted = ((failed and not passed and not timed_out) if expected_failure
                    else (passed and not failed and not timed_out and process.returncode == 0))
        records.append({"case": out.name, "expected_failure": expected_failure,
                        "returncode": process.returncode, "timed_out": timed_out,
                        "pass_marker": passed, "fail_marker": failed, "accepted": accepted})
        for line in output.splitlines():
            if line.startswith(("PASS ", "MODULE_ONE_")):
                say(out.name + ": " + line)
        if not accepted:
            raise RuntimeError("Unexpected result; inspect " + str(out / "console.log"))
        say(("REJECTED expected defect: " if expected_failure else "PASS image: ") + out.name)

    success = False
    error = None
    try:
        say("Results: " + str(run_dir))
        for opt in ["-O0", "-O2"]:
            boot(build(opt[1:], opt))
        if args.negative_controls:
            original = sources[1].read_text()
            mutations = {
                "missing-sepc-write": ("    csrw sepc, s1", "    nop # deliberate test-copy defect"),
                "missing-s0-restore": ("    LOAD x8, 8*REGBYTES(sp)", "    nop # deliberate test-copy defect"),
            }
            for label, (old, new) in mutations.items():
                if original.count(old) != 1:
                    raise RuntimeError("Cannot apply unambiguous mutation: " + label)
                mutant = run_dir / (label + ".S")
                mutant.write_text(original.replace(old, new))
                boot(build(label, "-O0", mutant), expected_failure=True)
        success = True
    except Exception as exc:
        error = str(exc)
        say("RUN FAILED: " + error)
    finally:
        after = hashes()
        unchanged = before == after
        say("Production tree unchanged: " + str(unchanged))
        success = success and unchanged
        report = {"success": success, "error": error, "production_unchanged": unchanged,
                  "source": str(source), "tools": tool_info, "cases": records,
                  "scope": "Actual entry.S/trapentry.S and SBI/console; fixture kern_init/trap; "
                           "real QEMU DTB is not parsed; no allocator/production handlers/grading."}
        (run_dir / "summary.json").write_text(json.dumps(report, indent=2))
        (run_dir / "commands.json").write_text(json.dumps(commands, indent=2))
        (run_dir / "output.txt").write_text("\n".join(transcript) + "\n")
        (results / "latest.txt").write_text(str(run_dir) + "\n")
    if not success:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
