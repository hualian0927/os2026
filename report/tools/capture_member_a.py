#!/usr/bin/env python3
"""Capture visible X11 windows of the actual saved command output, without image editing."""
from pathlib import Path
import hashlib
import json
import os
import sys

os.environ.setdefault("QT_QPA_PLATFORM", "xcb")
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QTextCursor
from PyQt5.QtTest import QTest
from PyQt5.QtWidgets import QApplication, QLabel, QTextEdit, QVBoxLayout, QWidget

ROOT = Path(__file__).resolve().parents[2]
EVIDENCE = ROOT / "report/evidence/2026-10-27-report"
IMAGES = ROOT / "report/images"


def main():
    if not (EVIDENCE / "collection.json").exists():
        raise RuntimeError("Run collect_member_a.py successfully before capturing")
    app = QApplication(sys.argv)
    if app.platformName() != "xcb":
        raise RuntimeError("A real visible X11 session is required")
    specs = [
        ("A01-foundation.png", "Foundation: actual startup, DTB, Best-Fit and SIE checks",
         "foundation-run.log", "FOUNDATION_C_ENTRY", None),
        ("A02-boot-gdb.png", "Guest GDB: startup arguments, stack and mappings",
         "boot-gdb.log", None, None),
        ("A03-module1.png", "Module one: O0/O2, 64 real Trap returns each",
         "module1-run.log", None, "missing-sepc-write:"),
        ("A04-frame-gdb.png", "Guest GDB: actual frame and edited register return",
         "frame-gdb.log", None, None),
        ("A05-negative-controls.png", "Expected rejections: two deliberately defective test copies",
         "module1-run.log", "missing-sepc-write:", None),
    ]
    records = []
    IMAGES.mkdir(exist_ok=True)
    for filename, title, name, start, end in specs:
        source = EVIDENCE / name
        body = source.read_text(errors="replace")
        if start:
            body = body[body.index(start):]
        if end:
            body = body[:body.index(end)]
        # Only display control-character escaping; raw files remain untouched.
        body = body.rstrip().replace("\x00", "\\x00")
        win = QWidget()
        win.setWindowTitle("Lab3 member A | Actual recorded output")
        win.setStyleSheet("QWidget {background:#18212b;color:#edf2f7;}"
                          "QTextEdit {background:#111820;border:1px solid #475569;}"
                          "QLabel {color:#96d4ff;padding:4px;}")
        layout = QVBoxLayout(win)
        header = QLabel(title + "\n2026-10-27 | Independent test fixture / recorded output | Source: " + name)
        header.setFont(QFont("DejaVu Sans", 11))
        layout.addWidget(header)
        edit = QTextEdit()
        edit.setReadOnly(True)
        edit.setAcceptRichText(False)
        edit.setLineWrapMode(QTextEdit.NoWrap)
        layout.addWidget(edit)
        for size in [18, 17, 16, 15]:
            font = QFont("DejaVu Sans Mono")
            font.setPixelSize(size)
            edit.setFont(font)
            edit.setPlainText(body)
            win.resize(2320, 1450)
            win.move(30, 20)
            win.show()
            app.processEvents()
            win.resize(2320, max(240, min(1450, int(edit.document().size().height()) + 125)))
            app.processEvents()
            if edit.verticalScrollBar().maximum() == 0 and edit.horizontalScrollBar().maximum() == 0:
                break
        if edit.verticalScrollBar().maximum() or edit.horizontalScrollBar().maximum():
            raise RuntimeError("Split the excerpt; it does not fit: " + filename)
        edit.moveCursor(QTextCursor.Start)
        edit.clearFocus()
        QTest.qWait(150)
        screenshot = win.grab()
        target = IMAGES / filename
        if screenshot.isNull() or not screenshot.save(str(target)):
            raise RuntimeError("Visible window capture failed: " + filename)
        records.append({"image": "report/images/" + filename,
                        "capture": "QWidget.grab of a visible X11 read-only log viewer; no image editing",
                        "source": str(source.relative_to(ROOT)), "start": start,
                        "end_exclusive": end,
                        "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                        "image_sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
                        "width": screenshot.width(), "height": screenshot.height()})
        win.close()
        print("CAPTURED " + filename, flush=True)
    (EVIDENCE / "screenshot-manifest.json").write_text(json.dumps(records, indent=2) + "\n")


if __name__ == "__main__":
    main()
