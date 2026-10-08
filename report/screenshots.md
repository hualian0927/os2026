# 实验一截图操作清单

本人提供的 11 张截图已保存到 `/home/lhl/os2026-lab1/report/images/` 并插入 [报告](report.md)。该工作目录对应 `lab1` 分支。以下保留采集步骤供复现；测试结果以报告中的实际截图和文本证据为准。

下列步骤沿用现有 `gdb-multiarch`，不修改 Makefile。每个终端开始时运行：

```bash
conda activate os2026
cd /home/lhl/os2026-lab1/code
```

开始一组新的调试前，退出上一组 QEMU 和 GDB。QEMU 退出方式：Ctrl+A，松开后按 X。不要同时让两个实例占用 1234 端口。

## 1. 01-build.png：重新编译

```bash
make clean
make
```

截图包含编译、`ld`、`objcopy` 输出且没有错误。`Nothing to be done` 只表示增量构建无需更新，不如重新编译的截图完整。

## 2. 02-reset-rom.png：复位入口

终端 A 执行原版调试命令：

```bash
make debug
```

终端 B 执行：

```bash
gdb-multiarch -ex 'file bin/kernel' -ex 'set arch riscv:rv64' \
    -ex 'target remote localhost:1234'
```

连接后：

```gdb
set pagination off
info registers pc
x/6i 0x1000
x/1gx 0x1018
```

截图保留 `pc = 0x1000`、六条指令以及 `0x1018` 中存放的 `0x80000000`。

## 3. 03-opensbi.png：单步进入固件

接着在同一个 GDB 会话执行：

```gdb
si 5
info registers pc a0 a1 a2 t0
si
info registers pc
```

截图包含跳转前 `pc = 0x1014`、`t0 = 0x80000000` 和跳转后 `pc = 0x80000000`。

原版参数不能在当前环境下完成后续交接。截完后输入 `detach`、`quit`，再退出终端 A 中的 QEMU。

## 4. 04-kernel-stack-a.png、04-kernel-stack-b.png：内核入口与练习一

这一张明确使用兼容启动命令，不写成原版 `make debug` 已成功。终端 A：

```bash
qemu-system-riscv64 -machine virt -nographic -bios default \
    -kernel bin/ucore.img -s -S
```

终端 B 使用前面的 `gdb-multiarch` 命令连接，然后：

```gdb
set pagination off
hbreak *0x80200000
continue
x/3i $pc
info registers pc sp ra
si 2
info registers sp
p/x &bootstacktop
si
info registers pc sp ra
```

截图需展示：到达 `kern_entry`，执行完整 `la` 后 `sp = 0x80203000`，执行 `tail` 后 `pc = 0x8020000a` 且 `ra` 不变。本次实际分为 `04-kernel-stack-a.png`、`04-kernel-stack-b.png` 两张，报告已引用。

## 5. 05-kernel-output.png：内核输出

在上一组 GDB 会话中执行：

```gdb
detach
quit
```

回到终端 A，截图保留以下内容：兼容启动命令（可使用终端滚动记录）、OpenSBI 的 `Domain0 Next Address = 0x80200000` 和 `(THU.CST) os is loading ...`。然后退出 QEMU。

也可以不调试，重新执行以下命令截取实际运行输出：

```bash
qemu-system-riscv64 -machine virt -nographic -bios default -kernel bin/ucore.img
```

## 6. 06-local-check.png：本地自检

```bash
make
python3 tools/check_lab1.py --gdb gdb-multiarch
```

截图保留命令、三行 `PASS` 和日志目录。脚本使用 `-kernel` 兼容启动，这是本地辅助检查，不是官方评分，也不是 `make grade` 成功。

如需展示格式化细节，可额外截图并命名 `06-console.png`：

```bash
tail -n 5 obj/check/console-O0.log
tail -n 5 obj/check/console-O2.log
```

这张可选截图应包含 `FMT` 和 `LAB1_CONSOLE_CHECK_PASS`，不是必须增加的截图。

## 7. 07-original-loader-a.png、07-original-loader-b.png：原版启动的实际限制

```bash
make qemu
```

截图保留该命令和 OpenSBI 的 `Domain0 Next Address = 0`。未出现 uCore 消息是本机原版启动参数的已知结果，不需要伪造成成功。截图后退出 QEMU。

## 8. 08-make-grade.png：原版评分入口现状

```bash
make grade
```

截图保留 `No rule to make target 'check'` 和返回终端的错误。原因是原版 Makefile 没有 `check` 目标，而此前保留的本地 `grade.sh` 调用了它；不要写成全部测试通过。

注意：该命令先执行清理，会删除 `bin/`、`obj/`。截图后执行 `make` 恢复构建产物；需要自检日志时再执行第 6 步。已经保存到 `report/evidence/` 的日志不受影响。

## 本次归档说明

共归档 11 张原始截图：编译 1 张，调试启动及复位 2 张，OpenSBI 跳转 1 张，内核入口与栈 2 张，内核消息 1 张，本地自检 1 张，原版启动 2 张，评分入口与恢复验证 1 张。所有图片均在报告中引用。

`05-kernel-output.png` 只展示内核消息，未展示完整启动参数；报告已说明证据范围。可选的 `06-console.png` 未提供，也没有制造占位图片或失效链接。用户已提供截图并要求整理推送，本轮按此完成提交。
