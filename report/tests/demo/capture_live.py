#!/usr/bin/env python3
"""Live GNU GDB/command frontend; captures its displayed QWidget.
X11 screen readback may be black under WSL; QWidget capture preserves the
actual live interface. No replay or terminal bitmap generation is used.
No log replay, terminal bitmap synthesis, inferior writes or production edits.
"""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import socket
import subprocess
import sys
import time

os.environ.setdefault('QT_QPA_PLATFORM', 'xcb')
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QTextCursor
from PyQt5.QtTest import QTest
from PyQt5.QtWidgets import QApplication, QLabel, QTabWidget, QTextEdit, QVBoxLayout, QWidget

HERE = Path(__file__).resolve().parent
LAB = HERE.parents[2]
SOURCE = LAB/'code'
OUT = Path(os.environ.get('LAB4_DEMO_EVIDENCE', LAB/'report/evidence/2026-11-10-demo-visible'))
IMAGES = Path(os.environ.get('LAB4_DEMO_IMAGES', LAB/'report/images'))
spec = importlib.util.spec_from_file_location('build_helper', HERE.parent/'migration/run.py')
helper = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helper)


def sha(data): return hashlib.sha256(data).hexdigest()


class Screen:
    def __init__(self):
        self.app = QApplication(sys.argv)
        if self.app.platformName() != 'xcb' or not self.app.screens():
            raise RuntimeError('Visible X11 screen unavailable; manual captures required')
        self.win = QWidget()
        self.win.setWindowTitle('Lab4 | Live GNU GDB / QEMU frontend')
        self.win.setStyleSheet('QWidget{background:#17212b;color:#edf3fa;} '
                              'QTextEdit{background:#101820;border:1px solid #61758b;} '
                              'QLabel{color:#a2d6fc;padding:5px;}')
        layout = QVBoxLayout(self.win)
        self.title = QLabel('Connecting actual processes...')
        self.title.setWordWrap(True)
        layout.addWidget(self.title)
        self.tabs = QTabWidget()
        self.views = []
        for name in ['Live debugger / command stdout', 'Live QEMU stdout']:
            view = QTextEdit()
            view.setReadOnly(True)
            view.setAcceptRichText(False)
            view.setLineWrapMode(QTextEdit.WidgetWidth)
            self.views.append(view)
            self.tabs.addTab(view, name)
        layout.addWidget(self.tabs)
        self.footer = QLabel('Actual child-process output | independent fixture | no image editing')
        self.footer.setWordWrap(True)
        layout.addWidget(self.footer)
        self.win.resize(2320, 1200)
        self.win.move(30, 25)
        self.win.show()
        self.app.processEvents()
        self.records = []

    def append(self, index, text):
        edit = self.views[index]
        edit.moveCursor(QTextCursor.End)
        edit.insertPlainText(text.replace('\0', '\\0'))
        edit.ensureCursorVisible()

    def clear(self, index): self.views[index].clear()

    def capture(self, name, title, feed, index=0):
        self.title.setText(title+'\nIndependent test image / LIVE process; not complete Lab4 startup')
        self.footer.setText('Raw output: '+str(feed.path.relative_to(LAB))+
                            '\nOnly visible viewport captured; complete output preserved unchanged.')
        self.tabs.setCurrentIndex(index)
        edit=self.views[index]
        for size in [18,17,16,15,14]:
            font=QFont('DejaVu Sans Mono');font.setPixelSize(size)
            edit.setFont(font)
            self.win.resize(2320,1450)
            self.app.processEvents()
            height=max(420,min(1450,int(edit.document().size().height())+175))
            self.win.resize(2320,height)
            self.app.processEvents()
            if edit.verticalScrollBar().maximum()==0: break
        if edit.verticalScrollBar().maximum():
            raise RuntimeError('Split this live-output phase before capture: '+name)
        edit.moveCursor(QTextCursor.Start)
        edit.clearFocus()
        self.win.raise_()
        self.app.processEvents();QTest.qWait(180)
        picture=self.win.grab()
        captured=picture.toImage()
        colors={captured.pixel(x,y) for x in range(0,captured.width(),17)
                for y in range(0,captured.height(),13)}
        if len(colors)<8:
            raise RuntimeError('Window capture is blank; manual capture required: '+name)
        target=IMAGES/(name+'.png')
        if picture.isNull() or not picture.save(str(target)):
            raise RuntimeError('Actual live-window capture failed: '+name)
        raw=feed.path.read_bytes()
        self.records.append({'image':str(target.relative_to(LAB)), 'title':title,
            'capture':'QWidget.grab of current live debugger frontend; actual child-process stdout',
            'raw_output':str(feed.path.relative_to(LAB)), 'raw_prefix_bytes':len(raw),
            'raw_prefix_sha256':sha(raw),'image_sha256':sha(target.read_bytes()),
            'width':picture.width(),'height':picture.height()})
        print('CAPTURED',name,flush=True)


class Feed:
    def __init__(self, command, path, screen, pane=0):
        self.path=path;self.screen=screen;self.pane=pane
        path.parent.mkdir(parents=True,exist_ok=True)
        self.log=path.open('wb',buffering=0)
        self.process=subprocess.Popen([str(x) for x in command],env=helper.ENV,
            cwd=LAB.parent,stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        os.set_blocking(self.process.stdout.fileno(),False)
        path.with_suffix('.command.txt').write_text(shlex.join(str(x) for x in command)+'\n')
        self.pending=b''

    def pump(self):
        chunks=[]
        while True:
            try: data=os.read(self.process.stdout.fileno(),65536)
            except BlockingIOError: break
            if not data: break
            self.log.write(data);chunks.append(data)
        if chunks:
            data=b''.join(chunks)
            self.screen.append(self.pane,data.decode(errors='replace'))
            self.pending+=data
        self.screen.app.processEvents()
        return chunks

    def wait(self, timeout=15):
        started=time.monotonic()
        while self.process.poll() is None:
            self.pump()
            if time.monotonic()-started>timeout: raise TimeoutError(str(self.path))
            QTest.qWait(15)
        self.pump()
        return self.process.returncode

    def stop(self):
        helper.stop(self.process)
        self.pump();self.log.close()


def port():
    with socket.socket() as s:
        s.bind(('127.0.0.1',0));return s.getsockname()[1]


def checkpoint_setup(folder):
    return '''python
import gdb, pathlib, time
_checkpoint_dir = pathlib.Path(%r)
def capture_checkpoint(name):
    gdb.write("CAPTURE_CHECKPOINT " + name + "\\n")
    gdb.flush()
    (_checkpoint_dir / "request.txt").write_text(name)
    deadline = time.monotonic() + 25
    while not (_checkpoint_dir / ("ack-" + name)).exists():
        if time.monotonic() > deadline:
            raise RuntimeError("Live frontend did not acknowledge " + name)
        time.sleep(0.03)
end
''' % str(folder)


def setup(image, number):
    return f'''set pagination off
set confirm off
set style enabled off
set print pretty off
file {image / 'kernel'}
set architecture riscv:rv64
target remote 127.0.0.1:{number}
set trace-commands on
'''


def software_script(image, number):
    return setup(image,number)+'''hbreak page_insert
condition 1 la == 0x4100
continue
set $dbg_a = page
set $dbg_root = pgdir
set $dbg_va = la
set $dbg_free = free_area.nr_free
printf "BEFORE first mapping\\n"
p/x $pc
p/x boot_pgdir_pa
p/x (($satp & 0xfffffffffff) << 12)
p/x $dbg_root[0]
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER first mapping: root -> L1 -> L0 -> data PTE\\n"
set $dbg_l2 = $dbg_root[($dbg_va >> 30) & 511]
set $dbg_l1 = (pte_t *)((($dbg_l2 >> 10) << 12) + va_pa_offset)
set $dbg_mid = $dbg_l1[($dbg_va >> 21) & 511]
set $dbg_l0 = (pte_t *)((($dbg_mid >> 10) << 12) + va_pa_offset)
set $dbg_xslot = $dbg_l0 + (($dbg_va >> 12) & 511)
p/x $dbg_l2
p/x $dbg_mid
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free")) - 2
python capture_checkpoint("A02-mapping-created")
condition 1 la == 0x4000 && page == $dbg_a && perm == 0
continue
printf "BEFORE same-page remap\\n"
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER same-page remap: flags replaced, no extra reference\\n"
p/x *$dbg_xslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free")) - 2
python capture_checkpoint("A03-same-page-remap")
condition 1 la == 0x4000 && page != $dbg_a && perm == 2
continue
set $dbg_b = page
printf "BEFORE replacing X: A also mapped at Y\\n"
p $dbg_a->ref
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 2
finish
printf "AFTER replacing X: A remains referenced by Y\\n"
p/x *$dbg_xslot
p $dbg_a->ref
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 1
python assert int(gdb.parse_and_eval("$dbg_b->ref")) == 1
python capture_checkpoint("A04-shared-page-replaced")
disable 1
hbreak page_remove
condition 2 la == 0x5100
continue
set $dbg_yslot = $dbg_l0 + 5
printf "BEFORE removing Y (last reference to A)\\n"
p/x *$dbg_yslot
p $dbg_a->ref
p free_area.nr_free
finish
printf "AFTER removing Y\\n"
p/x *$dbg_yslot
p $dbg_a->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_a->ref")) == 0
condition 2 la == 0x4000
continue
printf "BEFORE removing X (last reference to B)\\n"
p $dbg_b->ref
finish
printf "AFTER removing X: data freed, two table pages remain\\n"
p/x *$dbg_xslot
p $dbg_b->ref
p free_area.nr_free
python assert int(gdb.parse_and_eval("$dbg_b->ref")) == 0
python assert int(gdb.parse_and_eval("free_area.nr_free")) == int(gdb.parse_and_eval("$dbg_free"))
python capture_checkpoint("A05-unmap-and-free")
delete breakpoints
detach
quit
'''


def sie_script(image, number):
    fixture=LAB/'report/tests/migration/foundation.c'
    lines=fixture.read_text().splitlines()
    def line(fragment): return next(i for i,s in enumerate(lines,1) if fragment in s)
    before=line('local_intr_save(outer);')
    saved=line('assert(outer == enabled')
    restored=line('assert(!!(read_csr(sstatus) & SSTATUS_SIE) == enabled);')
    text=setup(image,number)+'''hbreak kern_init
continue
delete 1
python capture_checkpoint("CLEAR_QEMU")
'''
    for n in [before,saved,restored]: text+=f'hbreak {fixture}:{n}\n'
    for flag in [0,1]:
        for bp in [2,3,4]:text+=f'condition {bp} enabled == {flag}\n'
        text+='continue\nprintf "BEFORE local_intr_save\\n"\np enabled\np/x ($sstatus & 2)\np/x $sie\np/x $stvec\n'
        text+=f'python assert int(gdb.parse_and_eval("$sie")) == 0\npython assert int(gdb.parse_and_eval("$sstatus & 2")) == {flag*2}\n'
        text+='continue\nprintf "AFTER save: flag preserves original state\\n"\np outer\np/x ($sstatus & 2)\n'
        text+=f'python assert int(gdb.parse_and_eval("outer")) == {flag}\npython assert int(gdb.parse_and_eval("$sstatus & 2")) == 0\n'
        text+='continue\nprintf "AFTER local_intr_restore\\n"\np outer\np/x ($sstatus & 2)\np/x $sie\n'
        text+=f'python assert int(gdb.parse_and_eval("$sstatus & 2")) == {flag*2}\n'
        text+=f'python capture_checkpoint("A0{6+flag}-SIE-original-{flag}")\n'
    return text+'delete breakpoints\ndetach\nquit\n'


def run_session(screen, image, name, script_builder, titles):
    folder=OUT/name;folder.mkdir()
    number=port()
    console=Feed(helper.qemu_args(image)+['-S','-gdb',f'tcp:127.0.0.1:{number}'],
                 folder/'qemu.stdout.log',screen,1)
    body=script_builder(image,number)
    (folder/'manual.gdb').write_text('\n'.join(s for s in body.splitlines() if 'capture_checkpoint(' not in s)+'\n')
    script=folder/'session.gdb'
    script.write_text(checkpoint_setup(folder)+body)
    debugger=Feed(['gdb-multiarch','-q','-nx','-batch','-x',str(script)],folder/'gdb.stdout.log',screen)
    screen.clear(0);screen.clear(1)
    handled=set();started=time.monotonic()
    try:
        while debugger.process.poll() is None:
            console.pump();debugger.pump()
            request=folder/'request.txt'
            if request.exists():
                label=request.read_text().strip()
                if label not in handled:
                    console.pump();debugger.pump()
                    if label=='CLEAR_QEMU':screen.clear(1);screen.clear(0)
                    else:
                        screen.capture(label,titles[label],debugger)
                        screen.clear(0)
                    handled.add(label)
                    (folder/('ack-'+label)).touch()
            if time.monotonic()-started>100:raise TimeoutError('GDB session '+name)
            QTest.qWait(15)
        debugger.pump()
        if debugger.process.returncode:raise RuntimeError('GDB failed; see '+str(debugger.path))
        console.wait(6)
        output=console.path.read_bytes().replace(b'\0',b'').decode(errors='replace')
        expected='FOUNDATION_PASS' if name=='sie' else 'MAPPING_PASS'
        if expected not in output:raise RuntimeError('Independent fixture did not finish: '+name)
        if name=='sie':screen.capture('A01-memory-foundation','Current-source memory foundation / real DTB / allocator / handlers',console,1)
    finally:debugger.stop();console.stop()


def run_checks_live(screen, mode, filename):
    screen.clear(0)
    argv=['/home/lhl/anaconda3/envs/os2026/bin/python3',str(LAB/'report/tests/page_mapping/run.py'),
          '--mode',mode,'--evidence-dir',str(OUT/('live-'+mode))]
    feed=Feed(argv,OUT/(filename+'.stdout.log'),screen)
    try:
        if mode=='extended':
            started=time.monotonic();software_taken=False
            while feed.process.poll() is None:
                # Keep all bytes raw; capture live stdout at a genuine completion marker.
                chunks=feed.pump()
                if not software_taken and b'PASS software-O2\n' in feed.pending:
                    screen.capture('A08-software-tests','Current-source software / OOM / index-boundary checks',feed)
                    screen.clear(0);software_taken=True
                if time.monotonic()-started>45:raise TimeoutError('Live checks')
                QTest.qWait(15)
            feed.pump()
            if feed.process.returncode:raise RuntimeError('Extended checks failed')
            screen.capture('A09-real-access-faults','Real protection/deletion faults / separate independent images',feed)
        else:
            code=feed.wait(20)
            if code!=1 or b'MAPPING_FAIL' not in feed.path.read_bytes():
                raise RuntimeError('Known generated-path failure was not reproduced')
            screen.capture('A10-known-generated-failure','Known failure: production-generated non-leaf U bit / NOT a PASS',feed)
    finally:feed.stop()


def main():
    if OUT.exists():raise RuntimeError('Keep prior evidence intact; choose a new demo directory')
    OUT.mkdir(parents=True);IMAGES.mkdir(parents=True,exist_ok=True)
    before={str(p.relative_to(SOURCE)):sha(p.read_bytes()) for p in SOURCE.rglob('*')
            if p.is_file() and not any(x in ('obj','bin') for x in p.relative_to(SOURCE).parts)}
    (OUT/'source-before.json').write_text(json.dumps(before,indent=2)+'\n')
    screen=Screen()
    try:
        software=helper.build(OUT/'software-image',SOURCE,
            [LAB/'report/tests/page_mapping/check.c',LAB/'report/tests/page_mapping/access_probes.S',
             SOURCE/'kern/mm/default_pmm.c',SOURCE/'tools/test_lab4.c'],
            opt='-O0',trap=True,defines=['-DCHECK_MODE=0'])
        foundation=helper.build(OUT/'foundation-image',SOURCE,
            [LAB/'report/tests/migration/foundation.c',SOURCE/'kern/mm/default_pmm.c',
             SOURCE/'kern/trap/trap.c',SOURCE/'kern/driver/clock.c'],opt='-O0',trap=True)
        run_session(screen,software,'mapping',software_script,{
            'A02-mapping-created':'Software bookkeeping: actual page-table path / first insertion',
            'A03-same-page-remap':'Software bookkeeping: same page, flags changed, reference unchanged',
            'A04-shared-page-replaced':'Software bookkeeping: shared A remains live when X is replaced by B',
            'A05-unmap-and-free':'Software bookkeeping: last references removed; table pages retained'})
        run_session(screen,foundation,'sie',sie_script,{
            'A06-SIE-original-0':'Controlled interrupt state: original SIE=0, sie=0 throughout',
            'A07-SIE-original-1':'Controlled interrupt state: original SIE=1, sie=0 throughout'})
        run_checks_live(screen,'extended','extended')
        run_checks_live(screen,'generated','generated')
    finally:
        after={str(p.relative_to(SOURCE)):sha(p.read_bytes()) for p in SOURCE.rglob('*')
               if p.is_file() and not any(x in ('obj','bin') for x in p.relative_to(SOURCE).parts)}
        manifest={'production_unchanged':before==after,'screenshots':screen.records,
                  'scope':'Actual live debugger/QEMU/command frontend, independent images; generated path remains failed'}
        (OUT/'screenshot-manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
        screen.win.close()
        if before!=after:raise RuntimeError('Production changed during capture')
    print('CAPTURE COMPLETE:',len(screen.records),'actual visible-window screenshots',flush=True)


if __name__=='__main__':main()
