#!/usr/bin/env python3
"""Isolated syscall ABI/forwarding checks; never supplies production backends."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import tempfile
import argparse

HERE = Path(__file__).resolve().parent
LAB = HERE.parents[1]
SOURCE = LAB / 'lab5_new'
if not SOURCE.is_dir():
    SOURCE = LAB.parent / 'code'
EVIDENCE = LAB / 'evidence/2026-11-23-module-one'
spec = importlib.util.spec_from_file_location('foundation_runner', HERE.parent / 'foundation/run.py')
foundation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(foundation)


def manifest():
    return {str(p.relative_to(SOURCE)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in SOURCE.rglob('*') if p.is_file()
            and not {'obj', 'bin'} & set(p.relative_to(SOURCE).parts)}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--timer-only', action='store_true', help='real U spin and hardware timer, with schedule observer')
    args = parser.parse_args()
    EVIDENCE.mkdir(exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=EVIDENCE))
    print('Evidence:', run, flush=True)
    before = manifest()
    results = {}
    summary = {'commands': foundation.COMMANDS, 'results': results}
    extra_flags = ['-I' + str(SOURCE / 'kern/syscall'), '-march=rv64gc']
    common = [SOURCE / 'kern/trap/trap.c', SOURCE / 'kern/trap/trapentry.S',
              SOURCE / 'kern/mm/vmm.c', SOURCE / 'kern/driver/clock.c']
    try:
        for opt in (['-O2'] if args.timer_only else ['-O0', '-O2']):
            if not args.timer_only:
                interface = foundation.build(run / ('interface' + opt), HERE / 'interface.c',
                    [*common, SOURCE / 'tools/test_lab5.c'], opt, defines=extra_flags)
                results['interface' + opt] = foundation.boot(interface, 'INTERFACE_PASS:')
                print('PASS actual dispatcher/trap/VMA helper + unchanged syscall assertions', opt, flush=True)

            user = run / (('timer' if args.timer_only else 'user') + opt)
            user.mkdir()
            flags = ['-mcmodel=medany', '-march=rv64gc', '-mno-relax', '-msmall-data-limit=0',
                     '-std=gnu99', opt, '-g', '-fno-builtin', '-nostdinc', '-fno-stack-protector',
                     '-I' + str(SOURCE / 'libs'), '-I' + str(SOURCE / 'user/libs')]
            for name, source in [('probe', HERE / 'user_probe.c'),
                                 ('wrapper', SOURCE / 'user/libs/syscall.c'),
                                 ('raw', HERE / 'user_raw.S')]:
                foundation.command(['riscv64-unknown-elf-gcc', *flags, '-c', source,
                                    '-o', user / (name + '.o')], user / 'user-build.log')
            script = user / 'user.ld'
            script.write_text('ENTRY(user_start)\nSECTIONS { . = 0x400000; .text : { '
                              '*probe.o(.text*) *(.text*) } .rodata : { *(.rodata*) } '
                              '.data : { *(.sdata*) *(.data*) } .bss : { *(.sbss*) *(.bss*) } }\n')
            foundation.command(['riscv64-unknown-elf-ld', '-T', script, '-o', user / 'user.elf',
                                user / 'probe.o', user / 'wrapper.o', user / 'raw.o'], user / 'user-build.log')
            foundation.command(['riscv64-unknown-elf-objcopy', '-O', 'binary',
                                user / 'user.elf', user / 'user.bin'], user / 'user-build.log')
            # The entry must match the raw image's start; there is no ELF loader.
            symbols = subprocess.check_output(['riscv64-unknown-elf-nm', str(user / 'user.elf')],
                                               text=True, env=foundation.ENV)
            assert any(line.split() == ['0000000000400000', 'T', 'user_start']
                       for line in symbols.splitlines()), symbols
            embed = user / 'embed.S'
            embed.write_text('.section .rodata\n.balign 8\n.globl user_binary_begin,user_binary_end\n'
                             'user_binary_begin:\n.incbin "' + str(user / 'user.bin') + '"\n'
                             'user_binary_end:\n')
            out = foundation.build(user, HERE / 'user_kernel.c',
                [*common, SOURCE / 'kern/syscall/syscall.c', SOURCE / 'kern/mm/default_pmm.c',
                 HERE.parent / 'foundation/user.S', embed], opt,
                defines=extra_flags + (['-DTEST_TIMER_SPIN'] if args.timer_only else []))
            results[('timer' if args.timer_only else 'user') + opt] = foundation.boot(
                out, 'USER_TIMER_PASS:' if args.timer_only else 'USER_SYSCALL_PASS:',
                timeout=2 if args.timer_only else 8)
            assert (user / 'console.log').read_text().count('^') == 16
            print('PASS real U spin -> hardware timer -> production scheduling request' if args.timer_only
                  else 'PASS actual user wrappers -> U ecall -> production trap/syscall -> sret', opt, flush=True)
    except Exception as exc:
        summary['error'] = str(exc)
        raise
    finally:
        summary['production_unchanged_by_tests'] = before == manifest()
        summary['source_manifest'] = manifest()
        (run / 'summary.json').write_text(json.dumps(summary, indent=2))
        (EVIDENCE / 'latest.txt').write_text(str(run) + '\n')
    assert summary['production_unchanged_by_tests']


if __name__ == '__main__':
    main()
