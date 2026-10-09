# Lab 1：最小可执行内核

作者：2412351－李华濂。实验分支为 `lab1`，报告见 [report.md](../report/report.md)。

## 编译与运行

```bash
conda activate os2026
cd /home/lhl/os2026-lab1/code
make
make qemu
```

内核输出 `(THU.CST) os is loading ...` 后进入无限循环。退出 QEMU：按 Ctrl+A，松开后按 X。

## 双终端调试

两个终端都先激活 `os2026` 并进入 `code/`。终端一运行：

```bash
make debug
```

终端二运行：

```bash
make gdb
```

Makefile 默认调用已安装的 `gdb-multiarch`，连接 QEMU 的 1234 端口，并加载 `bin/kernel` 符号。`make debug` 自动构建所需镜像，启动后暂停 CPU，等待调试器。

## 本地自检

```bash
make check
# 或清理、重新编译后自检：
make grade
```

检查启动跟踪、栈初始化、尾跳转、格式化输出及字符计数；这是本地自检，不是课程官方评分。日志位于 `obj/check/`。

运行需要 RISC-V GCC/binutils、Make、QEMU 和支持 RISC-V 的 GDB。`environment.yml` 管理 Python 依赖，本机 `gdb-multiarch` 位于 `os2026` 环境的 `bin/`。这些系统工具不由该 yml 自动安装。
