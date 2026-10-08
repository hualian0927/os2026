# 文本验证证据

采集日期：2026-10-08。本人提供的 11 张原始截图已归档到 [images/](../images/) 并插入报告，文本日志用于补充截图未展示的完整运行条件。

- `current/`：本次交付的 `code/` 源码，原版 Makefile，现有 `gdb-multiarch`。
- `original/`：从本机 `lab1.zip` 解压的全部原始源文件，逐字节核对后在临时目录编译；`source-verification.txt` 记录归档摘要和核对结果。

## 运行条件

| 文件 | 采集方式及含义 |
|------|----------------|
| `current/build.log` | `make` 成功 |
| `current/make-grade.log` | `make grade` 缺少 `check` 目标，实际退出码 2 |
| `current/make-gdb.log` | 原版 `make gdb` 找不到指定名称的调试器 |
| `current/qemu-original-makefile.log` | `make qemu`，3 秒以内的启动观察，随后主动结束；Next Address 为 0 |
| `current/gdb-loader.log`、`current/qemu-loader-debug.log` | 原版 loader 参数，以独立 Unix socket 接入；设置入口断点后约 4 秒主动中断，未命中入口 |
| `current/local-check.log` | 重新 `make` 后执行 `python3 tools/check_lab1.py --gdb gdb-multiarch` 的成功结果 |
| `current/gdb-startup.log`、`current/qemu.log` | 上述脚本采用 `-kernel` 的兼容启动对照 |
| `current/console-O0.log`、`current/console-O2.log` | 当前源码的独立格式化测试内核，不是正式内核的正常输出 |
| `current/console-build.log` | 独立测试内核的真实编译命令 |
| `current/symbols.txt`、`current/kernel-disassembly.txt` | 当前编译产物的符号与反汇编 |
| `current/versions.txt` | 实际使用工具的版本 |
| `original/build.log`、`original/symbols.txt`、`original/kernel-disassembly.txt` | 未修改原版代码的编译和静态检查 |
| `original/make-debug.txt`、`original/make-gdb.txt` | 原版 Makefile 的 `make -n` 命令展开 |
| `original/qemu-original.log`、`original/gdb-original.log`、`original/qemu-original-debug.log` | 未修改原版代码和 loader 参数的启动记录，未到达内核 |
| `original/gdb-kernel-control.log`、`original/qemu-kernel-control.log` | 同一份原版镜像，仅外部启动命令换用 `-kernel`；入口、栈和输出验证成功 |

自动采集的 GDB 连接使用临时 Unix socket，避免与用户的 TCP 1234 会话冲突。GDB 日志中的 SIGINT 是采集程序为了结束未命中断点的观察主动发送的中断；QEMU 日志末尾的终止信息来自采集完成后的进程清理。

为使日志能作为文本查看，QEMU/OpenSBI 标题中的 NUL 字节转义为可见的 `\x00`；未更改地址、指令、测试结果或错误信息。临时目录可能已清理，日志中的路径用于说明当时的运行条件，不是需要上传的源文件。

`sha256.txt` 记录当前代码文件、文本证据及截图的 SHA-256 摘要；截图从本人消息附件原样保存，没有重绘、裁剪或修改显示内容。
