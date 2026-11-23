#!/usr/bin/env python3
"""Isolated Lab5 foundation checks. Never supplies production Lab5 modules."""
from pathlib import Path
import argparse, hashlib, json, os, re, shlex, shutil, socket, subprocess, tempfile
from zipfile import ZipFile
HERE=Path(__file__).resolve().parent
LAB=HERE.parents[1]
SOURCE=LAB/'lab5_new'
if not SOURCE.is_dir(): SOURCE=LAB.parent/'code'
ENV=os.environ.copy()
ENV['PATH']='/home/lhl/anaconda3/envs/os2026/bin:'+ENV.get('PATH','')
COMMANDS=[]
def command(argv,log,timeout=40):
    argv=list(map(str,argv));COMMANDS.append(argv)
    with log.open('ab') as out:
        out.write(('$ '+shlex.join(argv)+'\n').encode());out.flush()
        return subprocess.run(argv,stdout=out,stderr=subprocess.STDOUT,env=ENV,timeout=timeout,check=True)
def build(out,fixture,extra=(),opt='-O2',entry=None,dtb=True,defines=()):
    out.mkdir(parents=True,exist_ok=True)
    flags=['-mcmodel=medany','-std=gnu99',opt,'-Wall','-Wno-unused','-fno-builtin','-nostdinc','-fno-stack-protector','-ffunction-sections','-fdata-sections','-g']
    flags += ['-I'+str(SOURCE/d) for d in ['libs','kern/mm','kern/driver','kern/debug','kern/trap','kern/sync','kern/process','kern/schedule']]+list(defines)
    files=[entry or SOURCE/'kern/init/entry.S',SOURCE/'kern/driver/console.c',SOURCE/'kern/driver/intr.c',SOURCE/'kern/libs/stdio.c',SOURCE/'libs/printfmt.c',SOURCE/'libs/string.c']
    if dtb:files.append(SOURCE/'kern/driver/dtb.c')
    files += [fixture,*extra]
    objs=[]
    for i,f in enumerate(files):
        obj=out/f'{i}.o';command(['riscv64-unknown-elf-gcc',*flags,'-c',f,'-o',obj],out/'build.log');objs.append(obj)
    command(['riscv64-unknown-elf-ld','-m','elf64lriscv','-nostdlib','--gc-sections','-T',SOURCE/'tools/kernel.ld','-o',out/'kernel',*objs],out/'build.log')
    command(['riscv64-unknown-elf-objcopy','-O','binary',out/'kernel',out/'ucore.img'],out/'build.log')
    return out
def qemu(out,loader=False):
    a=['qemu-system-riscv64','-machine','virt','-m','128M','-smp','1','-nographic','-bios','default','-monitor','none']
    return a+(['-device','loader,file='+str(out/'ucore.img')+',addr=0x80200000'] if loader else ['-kernel',str(out/'ucore.img')])
def stop(p):
    if p.poll() is None:
        p.terminate()
        try:p.wait(timeout=2)
        except subprocess.TimeoutExpired:p.kill();p.wait(timeout=2)
def boot(out,marker=None,loader=False,timeout=8):
    a=qemu(out,loader);COMMANDS.append(a)
    log=out/('loader.log' if loader else 'console.log')
    (out/('loader-command.txt' if loader else 'qemu-command.txt')).write_text(shlex.join(a)+'\n')
    with log.open('wb') as f:
        p=subprocess.Popen(a,stdin=subprocess.DEVNULL,stdout=f,stderr=subprocess.STDOUT,env=ENV)
        timed=False
        try:p.wait(timeout=timeout)
        except subprocess.TimeoutExpired:timed=True
        finally:stop(p)
    text=log.read_text(errors='replace')
    if marker and (timed or p.returncode or marker not in text):raise RuntimeError(f'Boot failed: {log}')
    return {'log':str(log),'returncode':p.returncode,'timeout':timed,'marker_found':bool(marker and marker in text)}
def gdb(out,before=False):
    with socket.socket() as s:s.bind(('127.0.0.1',0));port=s.getsockname()[1]
    with (out/'gdb-qemu.log').open('wb') as f:
        p=subprocess.Popen(qemu(out)+['-S','-gdb',f'tcp:127.0.0.1:{port}'],env=ENV,stdout=f,stderr=subprocess.STDOUT,stdin=subprocess.DEVNULL)
        try:
            args=['gdb-multiarch','-q','-nx','-batch','-ex',f'file {out/"kernel"}','-ex','set arch riscv:rv64','-ex',f'target remote 127.0.0.1:{port}']
            if before:
                asm=subprocess.check_output(['riscv64-unknown-elf-objdump','-d',str(out/'kernel')],env=ENV,text=True)
                address=int(re.search(r'^\s*([0-9a-f]+):.*\bcsrw\s+satp',asm,re.M).group(1),16)-0xffffffff40000000
                obs=[f'hbreak *{address:#x}','continue','x/3i $pc','si','si','info registers pc satp scause sepc mcause mepc']
            else:obs=['hbreak *0x80200000','continue','info registers pc satp','hbreak kern_init','continue','info registers pc sp satp','x/gx ((char *)&boot_page_table_sv39)+2*8','x/gx ((char *)&boot_page_table_sv39)+511*8']
            for s in obs:args+=['-ex',s]
            command(args+['-ex','detach'],out/'gdb.log',timeout=15)
        finally:stop(p)
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--baseline',action='store_true');ap.add_argument('--only',choices=['user','conflict','memory','s-context']);a=ap.parse_args()
    run=Path(tempfile.mkdtemp(prefix='run-',dir=LAB/'evidence/2026-11-23-foundation'))
    print('Evidence:',run,flush=True);summary={'commands':COMMANDS,'checks':{},'tools':{}}
    original={str(p.relative_to(SOURCE)):hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCE.rglob('*') if p.is_file() and not any(c in ('obj','bin') for c in p.relative_to(SOURCE).parts)}
    for tool in ['riscv64-unknown-elf-gcc','riscv64-unknown-elf-ld','qemu-system-riscv64','gdb-multiarch','python3']:
        summary['tools'][tool]={'path':shutil.which(tool,path=ENV['PATH']),'version':subprocess.check_output([tool,'--version'],env=ENV,text=True).splitlines()[0]}
    try:
        if a.baseline:
            with ZipFile(LAB/'lab5.zip') as z:(run/'original-entry.S').write_bytes(z.read('lab5_new/kern/init/entry.S'))
            out=build(run/'startup-before',HERE/'startup.c',entry=run/'original-entry.S')
            summary['checks']['loader-before']=boot(out,loader=True,timeout=2)
            summary['checks']['kernel-before']=boot(out,timeout=2)
            gdb(out,before=True)
            print('Original startup and loader evidence collected; no success asserted.',flush=True)
        elif a.only:
            if a.only=='user':
                for opt in ['-O0','-O2']:
                    out=build(run/('context-u'+opt),HERE/'user.c',[HERE/'user.S',SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S'],opt,defines=['-march=rv64gc'])
                    summary['checks']['context-u'+opt]=boot(out,'USER_TRAP_PASS:')
                    print('PASS actual U/S trap return '+opt,flush=True)
            elif a.only=='conflict':
                out=build(run/'framework-selftest',HERE/'foundation.c',[SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S'],defines=['-DRUN_FRAMEWORK_BOOT_CHECK'])
                summary['checks']['framework-conflict']=boot(out,'FRAMEWORK_CONFLICT_OBSERVED')
                print('OBSERVED original non-leaf U / hardware access conflict; NOT a full self-test pass',flush=True)
            elif a.only=='memory':
                for opt in ['-O0','-O2']:
                    out=build(run/('foundation'+opt),HERE/'foundation.c',[SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S'],opt)
                    summary['checks']['foundation'+opt]=boot(out,'FOUNDATION_PASS')
                    print('PASS foundation '+opt,flush=True)
                for case in range(1,7):
                    out=build(run/f'page-init-{case}',HERE/'foundation.c',[SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S'],dtb=False,defines=[f'-DFIXTURE_CASE={case}'])
                    summary['checks'][f'page-init-{case}']=boot(out,'EXPECTED_PANIC_PASS' if case<6 else 'NO_FREE_PAGE_PASS')
                    print('PASS isolated page_init',case,flush=True)
            elif a.only=='s-context':
                out=build(run/'context-s-O2',HERE/'context.c',[HERE/'probe.S',SOURCE/'kern/trap/trapentry.S'],dtb=False)
                summary['checks']['context-s-O2']=boot(out,'MODULE_ONE_PASS:')
                print('PASS S trap 64 rounds -O2',flush=True)
        else:
            out=build(run/'startup',HERE/'startup.c');gdb(out)
            summary['checks']['startup']=boot(out,'STARTUP_PASS')
            print('PASS adapted entry + real DTB + matched ELF/GDB',flush=True)
            for opt in ['-O0','-O2']:
                extra=[SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S']
                out=build(run/('foundation'+opt),HERE/'foundation.c',extra,opt)
                summary['checks']['foundation'+opt]=boot(out,'FOUNDATION_PASS')
                print('PASS memory/descriptors/unchanged allocator checks/SIE '+opt,flush=True)
                out=build(run/('context-s'+opt),HERE/'context.c',[HERE/'probe.S',SOURCE/'kern/trap/trapentry.S'],opt,dtb=False)
                summary['checks']['context-s'+opt]=boot(out,'MODULE_ONE_PASS:')
                print('PASS actual trapentry S-mode 64 traps '+opt,flush=True)
                out=build(run/('context-u'+opt),HERE/'user.c',[HERE/'user.S',*extra],opt,defines=['-march=rv64gc'])
                summary['checks']['context-u'+opt]=boot(out,'USER_TRAP_PASS:')
                print('PASS actual trapentry U/S return '+opt,flush=True)
            for case in range(1,7):
                out=build(run/f'page-init-{case}',HERE/'foundation.c',[SOURCE/'kern/mm/default_pmm.c',SOURCE/'kern/trap/trapentry.S'],dtb=False,defines=[f'-DFIXTURE_CASE={case}'])
                marker='EXPECTED_PANIC_PASS' if case<6 else 'NO_FREE_PAGE_PASS'
                summary['checks'][f'page-init-{case}']=boot(out,marker)
                print('PASS isolated page_init case',case,flush=True)
    finally:
        after={str(p.relative_to(SOURCE)):hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCE.rglob('*') if p.is_file() and not any(c in ('obj','bin') for c in p.relative_to(SOURCE).parts)}
        summary['source_unchanged_by_tests']=original==after
        (run/'summary.json').write_text(json.dumps(summary,indent=2))
        (LAB/'evidence/2026-11-23-foundation'/('baseline-latest.txt' if a.baseline else 'latest.txt')).write_text(str(run)+'\n')
if __name__=='__main__':main()
