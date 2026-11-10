#!/usr/bin/env python3
"""Independent migration checks, never builds missing Lab4 modules for them."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import socket
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
LAB = HERE.parents[2]
SOURCE = LAB / 'code'
BASELINE = LAB / 'report/baseline/lab4_new'
ENV = os.environ.copy()
ENV['PATH'] = '/home/lhl/anaconda3/envs/os2026/bin:' + ENV.get('PATH', '')
COMMANDS = []


def command(args, log, timeout=30, cwd=None):
    COMMANDS.append({'argv': [str(x) for x in args], 'cwd': str(cwd) if cwd else None})
    with log.open('ab') as f:
        f.write(('$ ' + shlex.join(str(x) for x in args) + '\n').encode())
        f.flush()
        return subprocess.run([str(x) for x in args], env=ENV, cwd=cwd,
                              stdout=f, stderr=subprocess.STDOUT,
                              timeout=timeout, check=True)


def build(out, source, extra, opt='-O2', defines=(), trap=False, dtb=True, entry=None):
    out.mkdir(parents=True, exist_ok=True)
    flags = ['-mcmodel=medany', '-std=gnu99', opt, '-Wall', '-Wno-unused',
             '-fno-builtin', '-nostdinc', '-fno-stack-protector',
             '-ffunction-sections', '-fdata-sections', '-g']
    flags += ['-I'+str(source/d) for d in ['libs','kern/mm','kern/driver',
              'kern/debug','kern/trap','kern/sync','kern/process','kern/schedule']]
    flags += list(defines)
    files = [entry or source/'kern/init/entry.S', source/'kern/driver/console.c',
             source/'kern/driver/intr.c',
             source/'kern/libs/stdio.c', source/'libs/printfmt.c', source/'libs/string.c']
    if dtb: files.append(source/'kern/driver/dtb.c')
    if trap: files.append(source/'kern/trap/trapentry.S')
    files += list(extra)
    objects = []
    for i, file in enumerate(files):
        obj = out/(str(i)+'.o')
        command(['riscv64-unknown-elf-gcc', *flags, '-c', file, '-o', obj], out/'build.log')
        objects.append(obj)
    command(['riscv64-unknown-elf-ld','-m','elf64lriscv','-nostdlib','--gc-sections',
             '-T',source/'tools/kernel.ld','-o',out/'kernel',*objects],out/'build.log')
    command(['riscv64-unknown-elf-objcopy','-O','binary',out/'kernel',out/'ucore.img'],out/'build.log')
    return out


def qemu_args(out, loader=False):
    return ['qemu-system-riscv64','-machine','virt','-m','128M','-smp','1',
            '-nographic','-bios','default','-monitor','none'] + (
        ['-device',f'loader,file={out / "ucore.img"},addr=0x80200000'] if loader else
        ['-kernel',str(out/'ucore.img')])


def stop(process):
    if process.poll() is None:
        process.terminate()
        try: process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill(); process.wait(timeout=2)


def boot(out, marker=None, loader=False, timeout=8):
    args = qemu_args(out, loader)
    COMMANDS.append({'argv':args,'cwd':None})
    (out/('qemu-loader.txt' if loader else 'qemu.txt')).write_text(shlex.join(args)+'\n')
    log = out/('console-loader.log' if loader else 'console.log')
    with log.open('wb') as f:
        process = subprocess.Popen(args,env=ENV,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT)
        timed_out = False
        try: process.wait(timeout=timeout)
        except subprocess.TimeoutExpired: timed_out = True
        finally: stop(process)
    output = log.read_bytes().replace(b'\0',b'').decode(errors='replace')
    result = {'returncode':process.returncode,'timed_out':timed_out,
              'marker':marker, 'marker_found': marker in output if marker else None}
    if marker and (not result['marker_found'] or timed_out or process.returncode):
        raise RuntimeError('Boot check failed: '+str(log))
    return result


def gdb_observe(out, source, before=True):
    asm = subprocess.check_output(['riscv64-unknown-elf-objdump','-d',str(out/'kernel')],env=ENV,text=True)
    match = re.search(r'^\s*([0-9a-f]+):.*\bcsrw\s+satp,',asm,re.M)
    if not match: raise RuntimeError('Cannot locate satp write')
    physical_pc = int(match.group(1),16)-0xffffffff40000000
    with socket.socket() as sock:
        sock.bind(('127.0.0.1',0)); port=sock.getsockname()[1]
    args=qemu_args(out)+['-S','-gdb',f'tcp:127.0.0.1:{port}']
    with (out/'gdb-qemu.log').open('wb') as f:
        process=subprocess.Popen(args,env=ENV,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT)
        try:
            observations = (['-ex',f'hbreak *{physical_pc:#x}','-ex','continue',
                '-ex','x/4i $pc','-ex','si','-ex','si','-ex','si',
                '-ex','info registers pc satp mcause mepc'] if before else
                ['-ex','hbreak *0x80200000','-ex','continue',
                 '-ex','info registers pc satp','-ex','hbreak kern_init',
                 '-ex','continue','-ex','info registers pc sp satp',
                 '-ex','x/gx ((char *)&boot_page_table_sv39)+2*8',
                 '-ex','x/gx ((char *)&boot_page_table_sv39)+511*8'])
            command(['gdb-multiarch','-q','-nx','-batch','-ex',f'file {out / "kernel"}',
                '-ex','set arch riscv:rv64','-ex',f'target remote 127.0.0.1:{port}',
                *observations,'-ex','detach'],out/'gdb.log',timeout=15)
        finally: stop(process)
    return (out/'gdb.log').read_text()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--startup-before',action='store_true')
    parser.add_argument('--gdb-after',action='store_true',
                        help='only verify the adapted startup and matched-ELF GDB path')
    args=parser.parse_args()
    results=LAB/'report/evidence/2026-11-10-migration'
    results.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=results))
    print('Evidence:',run,flush=True)
    def hashes():
        return {str(p.relative_to(SOURCE)):hashlib.sha256(p.read_bytes()).hexdigest()
                for p in SOURCE.rglob('*') if p.is_file()
                and not any(x in ('obj','bin') for x in p.relative_to(SOURCE).parts)}
    before_hashes=hashes()
    record={'source':str(SOURCE),'commands':COMMANDS,'checks':{},
            'source_sha256':before_hashes}
    for tool in ['riscv64-unknown-elf-gcc','riscv64-unknown-elf-ld','qemu-system-riscv64','gdb-multiarch','make','python3']:
        path=shutil.which(tool,path=ENV['PATH'])
        if not path: raise RuntimeError('Missing tool: '+tool)
        record.setdefault('tools',{})[tool]={'path':path,'version':subprocess.check_output([path,'--version'],env=ENV,text=True).splitlines()[0]}
    try:
        if args.startup_before:
            image=build(run/'startup-before',BASELINE,[HERE/'startup_probe.c'])
            record['checks']['loader-before']=boot(image,loader=True,timeout=1)
            text=gdb_observe(image,BASELINE)
            record['checks']['kernel-before-gdb']=text
            print(text,flush=True)
        else:
            image=build(run/'startup-after',SOURCE,[HERE/'startup_probe.c'])
            record['checks']['startup-after']=boot(image,'STARTUP_PROBE_PASS')
            print('PASS real entry -> real DTB -> fixture C entry',flush=True)
            if args.gdb_after:
                text=gdb_observe(image,SOURCE,before=False)
                record['checks']['adapted-startup-gdb']=text
                print(text,flush=True)
                return
            for opt in ['-O0','-O2']:
                image=build(run/('foundation'+opt),SOURCE,
                    [HERE/'foundation.c',SOURCE/'kern/mm/default_pmm.c',
                     SOURCE/'kern/trap/trap.c',SOURCE/'kern/driver/clock.c'],opt=opt,trap=True)
                record['checks']['foundation'+opt]=boot(image,'FOUNDATION_PASS')
                print('PASS foundation '+opt,flush=True)
                image=build(run/('context'+opt),SOURCE,
                    [HERE/'context.c',HERE/'probe.S'],opt=opt,trap=True,dtb=False)
                record['checks']['context'+opt]=boot(image,'MODULE_ONE_PASS:')
                print('PASS actual trapentry context '+opt,flush=True)
            for case in range(1,7):
                image=build(run/('page-init-case-'+str(case)),SOURCE,
                    [HERE/'foundation.c',SOURCE/'kern/mm/default_pmm.c',
                     SOURCE/'kern/trap/trap.c',SOURCE/'kern/driver/clock.c'],
                    trap=True,dtb=False,defines=['-DFIXTURE_CASE='+str(case)])
                marker='EXPECTED_PANIC_PASS' if case < 6 else 'NO_FREE_PAGE_PASS'
                record['checks']['page-init-case-'+str(case)]=boot(image,marker)
                print('PASS isolated page_init case '+str(case),flush=True)
    finally:
        record['source_unchanged_by_tests']=(before_hashes==hashes())
        (run/'summary.json').write_text(json.dumps(record,indent=2,ensure_ascii=False)+'\n')
        (results/'latest.txt').write_text(str(run)+'\n')
        if not record['source_unchanged_by_tests']:
            raise RuntimeError('Production source changed during independent checks')


if __name__=='__main__': main()
