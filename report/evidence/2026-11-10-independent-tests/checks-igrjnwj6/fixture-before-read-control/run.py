#!/usr/bin/env python3
"""Bounded module checks; links actual pmm.c and unchanged course PMM tests."""
import argparse
import hashlib
import importlib.util
import json
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
LAB = HERE.parents[1]
spec = importlib.util.spec_from_file_location('migration_fixture', HERE.parent/'migration/run.py')
helper = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helper)
SOURCE = helper.SOURCE


def hashes():
    return {str(p.relative_to(SOURCE)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in SOURCE.rglob('*') if p.is_file()
            and not any(x in ('obj','bin') for x in p.relative_to(SOURCE).parts)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=['all','extended','software','prebuilt','generated','framework','protect','unmap'],default='all')
    parser.add_argument('--evidence-dir', type=Path, default=LAB/'report/evidence/2026-11-10-page-mapping')
    args=parser.parse_args()
    evidence = args.evidence_dir.resolve()
    evidence.mkdir(parents=True,exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='checks-',dir=evidence))
    before = hashes()
    records = {'scope':'Actual pmm.c/allocator/entry/trapentry and unmodified tools PMM test; '
               'fixture kern_init/panic/unexpected-trap exit; no production thread stubs.',
               'source_sha256':before,'checks':{},'commands':helper.COMMANDS}
    print('Evidence:',run,flush=True)
    try:
        cases=[('software',0,opt) for opt in ['-O0','-O2']]
        cases += [('prebuilt',1,opt) for opt in ['-O0','-O2']]
        cases += [('generated',2,'-O2'),('framework',3,'-O2')]
        cases += [('protect',4,opt) for opt in ['-O0','-O2']]
        cases += [('unmap',5,opt) for opt in ['-O0','-O2']]
        for name,mode,opt in cases:
            if args.mode == 'extended':
                if name not in ('software','protect','unmap'): continue
            elif args.mode not in ('all',name): continue
            image=helper.build(run/(name+opt),SOURCE,
                [HERE/'check.c',HERE/'access_probes.S',SOURCE/'kern/mm/default_pmm.c',SOURCE/'tools/test_lab4.c'],
                opt=opt,trap=True,defines=['-DCHECK_MODE='+str(mode)])
            key=name+opt
            try:
                result=helper.boot(image,'MAPPING_PASS mode=',timeout=10)
                result['passed']=True
            except RuntimeError as error:
                result={'passed':False,'error':str(error)}
            output=(image/'console.log').read_bytes().replace(b'\0',b'').decode(errors='replace')
            if 'MAPPING_FAIL' in output: result['passed']=False
            records['checks'][key]=result
            for line in output.splitlines():
                if line.startswith(('PASS ','OBSERVED ','MAPPING_','PTE ','[Test ','check_pgdir','check_boot')):
                    print(key+': '+line,flush=True)
            print(('PASS ' if result['passed'] else 'FAIL ')+key,flush=True)
    finally:
        records['source_unchanged_by_tests']=(before==hashes())
        records['all_passed']=bool(records['checks']) and all(r['passed'] for r in records['checks'].values())
        (run/'checks.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
        (evidence/'latest-check.txt').write_text(str(run)+'\n')
        if not records['source_unchanged_by_tests']:
            raise RuntimeError('Source changed during independent checks')
    if not records['all_passed']: raise SystemExit(1)


if __name__ == '__main__': main()
