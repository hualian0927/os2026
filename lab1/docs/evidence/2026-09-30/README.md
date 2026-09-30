# 实验一验证证据（2026-09-30）

本目录保存完整清理重建后执行 `make grade` 的证据。命令在 Conda `os2026` 中运行，退出码为 0；这是本地自检，不是课程官方评分。

| 文件 | 内容 |
| --- | --- |
| [gdb-startup.log](gdb-startup.log) | 复位 ROM、OpenSBI、内核入口和栈初始化的实际调试过程 |
| [qemu.log](qemu.log) | 正式内核启动和控制台消息 |
| [console-O0.log](console-O0.log) | O0 独立输出测试 |
| [console-O2.log](console-O2.log) | O2 独立输出测试 |
| [console-build.log](console-build.log) | 独立测试内核的编译与链接命令 |
| [elf-layout.txt](elf-layout.txt) | ELF 头、节和程序段布局 |
| [symbols.txt](symbols.txt) | 实际符号地址 |
| [kernel-disassembly.txt](kernel-disassembly.txt) | 当前正式内核反汇编 |
| [versions.txt](versions.txt) | 运行时间与工具版本 |
| [sha256.txt](sha256.txt) | 参与验证的源码、脚本和构建结果摘要 |

自检通过启动、栈/尾跳转检查，以及 O0/O2 两组格式化输出和字符计数检查。当前 BSS 范围为零，日志明确记录，未将它描述为非空 BSS 清零测试。

日志中的 `/tmp/lab1-check-*` 是一次性测试目录，结束后自动删除；QEMU 的终止提示来自脚本完成检查后的正常清理。这里复制保存的证据不会被 `make clean` 删除。二进制构建结果不提交到仓库，可按源码重建；其摘要用于标识本次实际使用的文件。

为了使 GitHub 按文本显示，串口横幅中出现的 NUL 字节记为可见的 `\x00`；寄存器、指令和测试结果原样保留。
