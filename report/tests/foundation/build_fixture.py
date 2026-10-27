from pathlib import Path
import subprocess,os,argparse
p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('output');a=p.parse_args()
src=Path(a.source).resolve();out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
w=Path(__file__).resolve().parent
env=os.environ.copy();env['PATH']='/home/lhl/anaconda3/envs/os2026/bin:'+env['PATH']
flags=['-mcmodel=medany','-std=gnu99','-Wall','-Werror','-Wno-unused','-fno-builtin','-O2','-nostdinc','-fno-stack-protector','-ffunction-sections','-fdata-sections','-g']
flags += ['-I'+str(src/d) for d in ['libs','kern/mm','kern/driver','kern/debug','kern/trap']]
sources=['kern/init/entry.S','kern/mm/pmm.c','kern/mm/best_fit_pmm.c','kern/driver/dtb.c','kern/driver/console.c','kern/driver/intr.c','kern/libs/stdio.c','libs/printfmt.c','libs/string.c','libs/sbi.c']
with (out/'build.log').open('w') as log:
 def run(cmd):
  log.write('$ '+' '.join(cmd)+'\n');log.flush();subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
 objs=[]
 for i,s in enumerate([src/f for f in sources]+[w/'foundation_check.c']):
  obj=out/(str(i)+'.o');run(['riscv64-unknown-elf-gcc',*flags,'-c',str(s),'-o',str(obj)]);objs.append(str(obj))
 run(['riscv64-unknown-elf-ld','-m','elf64lriscv','-nostdlib','--gc-sections','-T',str(src/'tools/kernel.ld'),'-o',str(out/'kernel'),*objs])
 run(['riscv64-unknown-elf-objcopy','-O','binary',str(out/'kernel'),str(out/'ucore.img')])
 asm=subprocess.check_output(['riscv64-unknown-elf-objdump','--disassemble=kern_entry',str(out/'kernel')],text=True,env=env)
 (out/'entry-disassembly.txt').write_text(asm)
print('Built isolated fixture: '+str(out/'kernel'))
