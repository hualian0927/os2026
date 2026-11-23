# Lab5基础迁移独立检查

这些是独立镜像，不是完整Lab5或官方评分结果。代码入口为本目录的受控kern_init，链接实际lab5_new的entry.S、pmm.c、default_pmm.c和trapentry.S，未向正式源码添加空handler、sys_*、进程装载或退出替身。

## 执行

在/home/lhl/os2026执行，脚本会复用os2026工具路径并只清理自己创建的QEMU进程：

```bash
conda activate os2026
python3 report/tests/foundation/run.py --baseline
python3 report/tests/foundation/run.py --only memory
python3 report/tests/foundation/run.py --only s-context
python3 report/tests/foundation/run.py --only user
python3 report/tests/foundation/run.py --only conflict
```

不带参数运行会执行适配后启动、内存与S/U上下文的-O0/-O2、六个page_init边界检查。`--baseline`另外保留原入口和原device loader的失败证据。`--only conflict`有意观察未修改框架的真实访存故障，不能将其观测成功说成check_boot_pgdir通过。

## 检查范围

- startup.c：真实生产entry和DTB解析，独立C入口；GDB与对应ELF/镜像一致。
- foundation.c：直接包含实际pmm.c以调用静态page_init和原check函数，链接真实Best-Fit；检查描述符数量/位置、固件/内核/描述符保留、空闲页、原basic_check/default_check/check_pgdir、最小足够块、低地址平局、分割合并与页数恢复。
- allocation_budget仅在测试中替换管理器的alloc回调，实际分配仍委托真实算法；检查get_pte第一次/第二次分配失败和已有上级页表保留，不改变生产分配器。
- 原SIE=0/1与嵌套标志，实际alloc/free/count接口恢复原中断状态。sie来源位保持0，安装受控Trap入口后才测试SIE=1。
- context.c/probe.S：实际SAVE_ALL/RESTORE_ALL，64次S态Trap、全寄存器、sp、真实C调用、合法帧修改、epc和SIE/SPIE/SPP转换；驱动保存自身ABI状态。
- user.c/user.S：真实forkrets/Trap汇编，64次U态ecall后恢复U态，再一次受控修改帧返回S态；验证可信内核栈、用户栈读写、帧修改、C调用和页数恢复。它是专用汇编探针，不是用户ELF装载或生产syscall测试。
- U态探针显式构造合法Sv39测试页表，使用实际分配器和pte构造接口；它只隔离证明Trap机制，**不证明原get_pte生成的硬件映射已经正确**。测试镜像用rv64gc启用fence.i所需Zifencei，生产CFLAGS未改变。
- 非默认FIXTURE_CASE使用测试DTB getter输入，未测试畸形DTB解析；每种panic独立镜像，验证准确panic分支而不是一启动就中断其他测试。

## 不能据此声称通过的项目

这些基础镜像不能证明完整kern_init、进程创建/调度运行、用户ELF、生产syscall或fork/exec/wait/exit，以及官方make grade。生产模块一现已单独实现并检查，见[模块一独立检查](../module_one/README.md)；同伴模块二至四仍待实现，模块一探针也不证明这些后端业务。原页表自检/架构冲突另见[迁移说明](../../基础迁移与验证说明.md)，当前综合结论见[成员A交接](../../成员A阶段交接说明.md)。

日志在report/evidence/2026-11-23-foundation，每次记录命令、工具、ELF、镜像、console、summary.json。源码在测试期间的哈希保持不变。
