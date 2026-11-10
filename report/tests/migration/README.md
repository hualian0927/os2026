# Lab4依赖迁移独立检查

这是成员A迁移阶段的测试支架，不是Lab4模块一、二、三的生产实现，不修改正式源码或评分测试。

从仓库工作目录运行：

```bash
conda activate os2026
cd /home/lhl/os2026
python3 lab4/tests/migration/run.py
python3 lab4/tests/migration/run.py --gdb-after
```

`--startup-before`用于重现原包的启动配置问题，读取`lab4/baseline/lab4_new`，不会修改基线。所有模式均在`lab4/report/evidence/2026-11-10-migration/run-*`保存独立镜像、命令、原始输出及摘要；运行结束只停止自己启动的QEMU。重复运行产生新目录，不覆盖旧证据。

## 被测实现与替身边界

| 检查 | 真实生产模块 | 独立支架/未覆盖内容 |
|---|---|---|
| 启动观察 | `entry.S`、DTB解析、控制台及SBI | `startup_probe.c`提供测试C入口；不进入正式`kern_init/pmm_init` |
| foundation，-O0/-O2 | 实际`pmm.c`的`page_init`和页分配包装、完整Best-Fit及原包自测、`trapentry.S`、正式handler、时钟、真实DTB | `foundation.c`直接包含实际`pmm.c`以访问静态初始化；提供独立`kern_init`及panic出口；未使用的页表初始化路径由链接器删除，不提供假的page_insert/page_remove或线程函数 |
| context，-O0/-O2 | 实际`entry.S`、`trapentry.S`及控制台 | 从Lab3独立检查适配的`context.c/probe.S`提供受控trap(tf)；不测试正式handler，正式handler由foundation另测；不解析DTB或初始化物理页 |
| 预期panic，case 1～5 | 实际`page_init`与管理器 | 测试getter分别给出零基址、零长度、范围溢出、描述符空间不足、范围未覆盖内核；不链接DTB解析器，检查准确panic原因后退出 |
| 无完整空闲页，case 6 | 实际`page_init`和管理器 | 测试getter构造描述符恰好占满受管理区域；检查分配器仍无可分配页，零长度注册会被真实管理器断言拒绝 |

foundation还测试：扩展Page字段初始化、保留区、空闲页数恢复、SIE为0/1时包装恢复原状态、不同变量的嵌套保存恢复、16/32位断点和32位非法指令返回，以及至少102次正式时钟中断重装与返回。测试异步中断前先安装真实入口；SIE包装检查期间`sie=0`，使中断来源受控。

context每种优化级运行64次受控Trap，观察原sp、帧指针、通用寄存器、真实C调用破坏后的恢复、epc和部分GPR帧修改、S态返回及SIE/SPIE关系。它保留Lab3检查输出中的`MODULE_ONE_PASS`名称，这里指**Lab3的Trap模块一**，不代表已经实现Lab4页映射模块。Lab4的SAVE_ALL不初始化x0槽，因此只检查x1～x31的帧副本；真实x0返回值仍检查为0。

`--gdb-after`加载自己生成的匹配ELF，在物理入口和高地址C入口设置硬件断点，观察PC、sp、satp以及根项2和511。不把该独立镜像的地址当成未来正式内核的固定地址。

## 本轮实际结果与边界

首次完整通过的运行目录：

`../../report/evidence/2026-11-10-migration/run-xoikc2sx/`

共11个镜像检查通过：启动1个、foundation 2个、context 2个、隔离panic 5个、无空闲完整页1个。GDB适配后观察另存于`run-xn6b6ljf/`，修改前的启动观察在`run-h3h2cfz_/`。

最初测试支架漏链`intr.c`导致控制台依赖无法链接，已补齐真实`intr.c`后重跑；失败证据保存在`run-owqwt95f/`。这是支架依赖问题，不是生产代码的Trap或内存错误。

未验证：正式`kern_init → pmm_init → vmm_init → proc_init`启动链、Lab4三个功能模块、完整评分、用户态、多hart、任意嵌套Trap、FPU/向量现场及畸形DTB解析。无效内存输入用例验证的是getter接口后的校验，不是DTB解析器的健壮性。DEBUG_GRADE保留原包实现，本轮未运行完整评分中的该路径。
