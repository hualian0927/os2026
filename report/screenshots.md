# 实验一运行与截图说明

报告中的 11 张截图保存在 `images/`，包括编译、启动跟踪、输出、自检和排错过程。以下为修正配置后的复现命令。

两个终端都先执行：

```bash
conda activate os2026
cd /home/lhl/os2026-lab1/code
```

## 编译与启动

```bash
make clean
make
make qemu
```

`make` 完成编译、链接及镜像转换，`make qemu` 运行后输出 `(THU.CST) os is loading ...`。退出 QEMU：Ctrl+A，松开后按 X。

## 双终端调试

终端一：

```bash
make debug
```

终端二：

```bash
make gdb
```

GDB 中依次执行：

```gdb
info registers pc
x/6i 0x1000
x/1gx 0x1018
si 5
info registers pc a0 a1 a2 t0
si
info registers pc
hbreak *0x80200000
continue
x/3i $pc
info registers pc sp ra
si 2
info registers sp
p/x &bootstacktop
si
info registers pc sp ra
detach
quit
```

观察初始 PC、六条复位指令、OpenSBI 入口、内核断点、栈顶地址和尾跳转前后的寄存器。`detach` 后内核继续运行，QEMU 终端显示启动消息。

## 本地自检

```bash
make check
# 清理、重编译后自检：
make grade
```

两种命令均调用 `tools/check_lab1.py`。正常结果包含启动跟踪、O0 输出和 O2 输出的三项 PASS。`make grade` 是本地自检，不是课程官方评分。

## 已有截图对应关系

| 文件 | 内容 |
|------|------|
| `01-build.png` | 清理后编译、链接和镜像生成 |
| `02-debug-launch.png` | QEMU 启动后暂停等待 |
| `02-reset-rom.png` | 复位 PC、六条指令及下一阶段地址 |
| `03-opensbi.png` | 单步进入 OpenSBI |
| `04-kernel-stack-a.png`、`04-kernel-stack-b.png` | 内核入口、栈初始化及尾跳转 |
| `05-kernel-output.png` | 内核启动消息 |
| `06-local-check.png` | 直接运行 Python 自检的成功结果 |
| `07-boot-issue-a.png`、`07-boot-issue-b.png` | 配置过程中发现启动入口为 0 的排错记录 |
| `08-make-grade.png` | 配置过程中缺少 check 目标的排错记录 |

最后三张图片记录的是已解决的问题。2026-10-09 重新执行 `make qemu`、`make debug`、`make gdb` 和 `make grade` 均通过，文字记录见 [验证记录](evidence/2026-10-09/README.md)。现有图片保持不变，没有将报错画面修改成成功画面。
