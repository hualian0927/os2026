# Lab 1：最小可执行内核

交付分支：`lab1`。代码在本目录，报告见 [report.md](../report/report.md)，截图步骤见 [screenshots.md](../report/screenshots.md)。作者：2412351－李华濂。

## 本机构建与验证

```bash
conda activate os2026
cd /home/lhl/os2026-lab1/code
make
python3 tools/check_lab1.py --gdb gdb-multiarch
```

Makefile 保持 `lab1.zip` 原版，其他源文件沿用现有工作版本。当前 `make gdb` 指定的 `riscv64-unknown-elf-gdb` 未安装，应直接使用已有的 `gdb-multiarch`。原版没有 `check` 目标；保留的历史 `tools/grade.sh` 调用该目标，因此 `make grade` 当前会失败。上面的 Python 脚本是可直接运行的本地自检，不是官方评分；内部使用 `-kernel`，不能用它的 PASS 声称原版 loader 启动成功。

原版 `make qemu` / `make debug` 使用 loader 参数，在本机默认 OpenSBI 1.3 下显示 Next Address 为 0。保持 Makefile 不变的兼容运行命令：

```bash
qemu-system-riscv64 -machine virt -nographic -bios default -kernel bin/ucore.img
```

需要调试时在后面加 `-s -S`，另一终端运行：

```bash
gdb-multiarch -ex 'file bin/kernel' -ex 'set arch riscv:rv64' \
    -ex 'target remote localhost:1234'
```

`environment.yml` 只描述 Conda 的 Python 依赖，系统还需要 RISC-V GCC/binutils、Make 和 QEMU。已有 `gdb-multiarch` 放在本机 `os2026` 环境的 `bin/`，不是单靠该 yml 重建的工具。退出 QEMU：按 Ctrl+A，松开后按 X。
