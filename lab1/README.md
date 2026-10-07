# os2026 / lab1

李华濂的操作系统实验一。源码、报告、环境说明和验证证据全部存放在仓库的 `lab1/` 目录中，与后续实验隔离。

## 实验一：最小可执行内核

实验一涵盖最小 uCore 的构建与启动、入口汇编分析、QEMU/GDB 启动跟踪，以及 SBI 格式化输出链路。当前提交包含全部练习答案、源码、环境说明、提示词记录和真实验证证据。

- [正式实验报告](实验报告.md)
- [实际提示词与验证记录](docs/提示词与验证记录.md)
- [环境配置说明](docs/环境配置.md)
- [代码变更与验证范围](docs/代码完成说明.md)
- [本次验证证据](docs/evidence/2026-09-30/README.md)

## 构建与运行

本机使用 Ubuntu 24.04、RISC-V GCC 13.2.0、QEMU 8.2.2（OpenSBI 1.3）及支持 RISC-V 的 GDB 15.1。

```bash
conda activate os2026
cd lab1
make grade
make qemu
```

`make grade` 清理并重建内核，然后进行本地启动及输出自检；不代表课程官方评分。`make qemu` 输出 `(THU.CST) os is loading ...` 后进入无限循环，按 `Ctrl+A`，松开后按 `X` 退出。

调试时在两个终端分别运行 `make debug`、`make gdb`。可在 GDB 初始暂停时执行 `source tools/boot.gdb`，或按报告逐条跟踪。

新机器需要 RISC-V GCC/binutils、Make、QEMU、Python 和支持 RISC-V 的 GDB。`environment.yml` 只管理 Conda 依赖，并不包含系统编译器/QEMU；按 [环境说明](docs/环境配置.md) 准备完整工具链。若系统已安装多架构 GDB，也可直接运行 `make grade PYTHON=python3 GDB=gdb-multiarch`，不强制依赖 Conda。

## 提交约定

每次提交具体描述实际完成内容，标题使用：

```text
李华濂：完成实验一启动分析与GDB验证，补齐报告和环境说明
```

提交不附加 AI 署名。提示词与辅助工具的使用过程按课程要求记录在实验文档中。源代码中的原有作者、版权及许可说明保持原样。
