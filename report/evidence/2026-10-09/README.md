# 2026-10-09 验证记录

工作目录：`/home/lhl/os2026-lab1/code`。使用 Conda `os2026` 环境中的 Python 与 `gdb-multiarch`，Makefile 通过 `-kernel` 启动 QEMU。

| 文件 | 验证内容 |
|------|----------|
| `clean.log`、`build.log` | 执行 `make clean` 后重新 `make`，编译成功 |
| `make-qemu.log` | 实际执行 `make qemu`，观察到内核启动消息 |
| `make-gdb.log` | 实际执行 `make gdb`，通过 TCP 1234 连接 `make debug` 启动的 QEMU；随后执行 `source tools/boot.gdb` 完成寄存器和断点检查 |
| `make-debug-qemu.log` | 上述 `make debug` 的输出，GDB 分离后内核打印启动消息 |
| `make-grade.log` | 实际执行 `make grade`，清理、重新编译及三项自检均通过 |
| `gdb-startup.log`、`qemu.log` | 自检脚本内部通过独立 Unix socket 完成启动检查 |
| `console-O0.log`、`console-O2.log` | 独立测试内核的格式化输出与字符计数验证 |
| `console-build.log` | 独立测试内核的编译与链接命令 |

GDB 空提示符行末的空格已去除。QEMU 输出中 NUL 字节显示为 `\x00`，日志末尾的终止信号来自测试结束后的进程清理。只关闭本次验证启动的进程，没有操作已有用户会话。

本次修改不影响入口机器码和内核栈布局，验证结果仍为 `kern_entry = 0x80200000`、`bootstacktop = 0x80203000`、`kern_init = 0x8020000a`，尾跳转前后 `ra` 不变。BSS 范围长度为零，尚未覆盖非空 BSS 清零。
