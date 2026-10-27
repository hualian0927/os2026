# Lab3 功能模块一独立测试

测试链接正式目录的 `kern/init/entry.S`、`kern/trap/trapentry.S` 与实际链接脚本；不修改或复制重写被测 Trap 保存恢复逻辑。`probe.S` 中的普通函数序言仅保护测试驱动的调用者 ABI，Trap 返回的寄存器快照在该序言恢复之前取得。

## 运行前的行为预期

| 行为 | 观测方法 | 预期 |
|---|---|---|
| 实际帧与布局 | 静态偏移断言，C 检查传入指针、全部 32 槽 | tf=触发前 sp−288，原 sp 位于第 2 槽，16 字节对齐 |
| 通用寄存器恢复 | 汇编设置哨兵，在返回后立即快照；gp/tp 保留实际值 | 快照等于处理后的帧，而不是 C 的临时寄存器值 |
| C 调用的影响 | noinline C 调用、栈数组、计算及 caller-saved 显式覆盖 | 调用确实发生；Trap 返回现场不受影响 |
| 返回地址修改 | tf->epc 指向跨过非法指令的后继 | 返回选定位置，不重放断点或误执行非法指令 |
| 合法 GPR 修改 | 修改帧中的 a0、s0、s1、t0 | 修改值真实带回，其他寄存器保持 |
| 连续 Trap 与驱动 ABI | 64 次连续往返，检查三层 SP；外层检查 s0-s11 | 无栈漂移，普通调用的 callee-saved 状态保持 |
| 状态位 | 组合 SIE 初值、SPIE 翻转与 GPR 修改 | 保存时 SPP=1/SIE=0；返回 SIE=帧 SPIE、SPIE=1、SPP=0 |
| 诊断字段 | 修改帧 cause/badvaddr，读取返回硬件 CSR | 不机械写回诊断 CSR，sscratch 保持 0 |
| 敏感性检查 | 只在临时汇编副本分别删除 sepc 写回、s0 恢复 | 原预期不改，两份错误镜像均产生明确失败标记 |

## 可复现命令

```bash
conda activate os2026
cd /path/to/os2026-lab3
python3 report/tests/module1/run.py --source "$PWD/code" --negative-controls
```

分支根目录运行时必须显式传 `--source "$PWD/code"`；测试脚本没有改写原开发路径默认规则。

不加 `--negative-controls` 时只运行实际生产实现的 `-O0/-O2` 测试。每次创建独立 `results/run-*` 目录，`results/latest.txt` 保存最近一次结果位置。各镜像的编译命令、反汇编在 `build.log`，QEMU 命令在 `qemu-command.txt`，原始串口在 `console.log`，脚本原始输出在 `output.txt`，机器可读结果在 `summary.json`。运行前后比较正式代码目录全部文件的 SHA-256，基线在 `production-before.json`。

## 2026-10-27 实际执行结果

执行入口为上述带 `--negative-controls` 的命令，脚本退出码为 0。结果目录为 [results/run-stt3fs9o](results/run-stt3fs9o/)，脚本原始输出为 [output.txt](results/run-stt3fs9o/output.txt)，结构化结论为 [summary.json](results/run-stt3fs9o/summary.json)。本轮未发现被测正式实现的错误，未修改正式代码。

| 检查项 | 实际结果 |
|---|---|
| tf 指针、原 sp、帧布局与对齐 | O0/O2 各 64 次通过 |
| 全部 GPR 槽位、ra 与 gp/tp 保持 | O0/O2 各 64 次通过 |
| noinline C 调用、栈计算及 caller-saved 覆盖 | O0/O2 各 64 次通过；反汇编确认 trap → inspect_frame → c_work 存在真实调用 |
| 修改 epc 并跳过非法指令 | O0/O2 各 64 次通过 |
| 修改 a0/s0/s1/t0 并带回 | O0/O2 各 32 次修改、32 次不修改，均通过 |
| 连续 Trap 栈稳定与驱动 callee-saved ABI | O0/O2 各 64 次通过 |
| SPP 与 SIE/SPIE 转换、sscratch | O0/O2 各 64 次通过，未使用整个 sstatus 相等断言 |
| cause/badvaddr 不机械写回 | O0/O2 各 64 次通过 |
| 删除 sepc 写回的错误副本 | 明确失败：round=0、trap_count=1 时再次进入同一断点，被 trap_count == round_id 断言检出 |
| 删除 s0 恢复的错误副本 | 明确失败：返回寄存器快照与预期不一致，被 observed[i] == expected 断言检出 |
| 正式目录前后哈希 | 完全一致；实现、SAVE_ALL、结构体、构建配置和评分测试均未改动 |

原始串口：[O0](results/run-stt3fs9o/O0/console.log)、[O2](results/run-stt3fs9o/O2/console.log)、[缺少 sepc 写回](results/run-stt3fs9o/missing-sepc-write/console.log)、[缺少 s0 恢复](results/run-stt3fs9o/missing-s0-restore/console.log)。四个 QEMU 进程均正常关闭、没有超时。由于测试 panic 通过 SBI 关机，负面对照的 QEMU 退出码也为 0；脚本根据明确的失败标记判定检出错误，不把退出码 0 单独视为通过。

实际工具为 `/usr/bin/riscv64-unknown-elf-gcc` 13.2.0、GNU ld/objcopy 2.42、`/usr/bin/qemu-system-riscv64` 8.2.2，固件串口显示 OpenSBI 1.3。脚本优先将既有 os2026 环境的 bin 加入 PATH，没有安装工具。详细信息在 [tools.json](results/run-stt3fs9o/tools.json)。GDB 不参与本轮自动验证。

## 替身与边界

- 真实 QEMU virt / OpenSBI、单 hart、128 MiB，使用正式启动汇编和 SBI 控制台；不是模拟 Python 页帧测试。
- `check.c` 仅为独立镜像提供 `kern_init()`、`trap(tf)` 和 panic 入口。trap 替身只检查受控断点帧、选定返回位置、修改测试字段，不实现通用异常或时钟处理。
- QEMU 提供真实 DTB，但该测试不解析它，也没有假 DTB getter；不调用 PMM，不检验内存迁移。
- `sie=0`，即使组合测试将全局 SIE 设为 1，也没有异步来源；不把结果当作连续时钟中断验证。
- 使用真正的 32 位断点，未测试压缩断点、U 态换栈、多 hart、嵌套 Trap、浮点/向量现场或非法返回帧。
- 两个负面对照各运行在独立 QEMU 镜像，10 秒超时；只终止脚本自己启动的进程。超时本身不作为检出目标错误的证据。
- `gp/tp` 不被驱动主动改写，动态证据仅证明本场景中值保持，不能声称能识别其所有漏恢复错误。
- 不运行、不修改正式评分测试，结果不等于整个 Lab3 通过。
