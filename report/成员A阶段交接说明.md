> 提交前阶段记录：其中目录、摘要和“尚未提交”描述对应本机整理当时；仓库布局及本次提交范围见[交付说明](交付说明.md)。

# Lab4成员A阶段交接说明

交接人：李华濂，2412351。接续成员：肖子昂2412181、杨昊冉2312115。整理日期：2026-11-10。

## 1. 交接状态与版本定位

我已完成Lab2/3必要依赖迁移、启动调试适配、模块一实现、独立检查和Challenge材料。当前模块二、三仍为双人组任务，没有提供空进程函数绕过链接。正式构建仍缺`proc_init/cpu_idle`；另有自动建表的非叶子U位冲突未解决。因此本次交接是可复查的阶段材料，不是正式Lab4已经验收通过。

当前工作目录为`/home/lhl/os2026/lab4/lab4_new`；报告在同级`report/`。尚未创建本阶段Git提交，不使用其他分支的HEAD冒充Lab4版本号。本次以文件SHA256和配对ELF摘要定位版本，未改远端、Git历史、Lab1～3或工程目录结构。

原包SHA256：`7a92f243f88ca9a4ec78b52e554a14d78a41bbf6a79a1275f6cc929aaebceba2`。原包展开基线在`../baseline/lab4_new`，逐文件摘要在[manifest.json](baseline/manifest.json)。

以下8个正式文件与原包不同；其他正式文件包含同伴实现区在内保持原包状态。与迁移阶段7文件相比，模块一又修改了pmm.c，并仅给pmm.h的flush_tlb增加memory clobber，所以当前净差异为8个文件。

| 正式文件（相对lab4_new） | SHA256 |
|---|---|
| Makefile | `3e5b10eab2983b131002f7d56a56960a8c7f687598ecd18923b08b05729f6815` |
| kern/init/entry.S | `bd9bd573991597eac474655a81459f5f3c9aecfa33dcc8a686c6a8b25b545880` |
| kern/mm/default_pmm.c | `5d31f4a2cff67dc13ea76338d69240b1e231f2f79cb654facb7ae59d6a7ff9d8` |
| kern/mm/pmm.c | `e2ebef3c1c82c8d336b04b61b34517c8531e52f425f2071b3609788ec1e5450a` |
| kern/mm/pmm.h | `045f073bfc709678654698c0bcaabf984443ee7b43e83104490191cf265996b6` |
| kern/trap/trap.c | `d35b8fd1ae4ae59a6bbeb38deb08d0a2c3bc956cd7b5b4461c0f4f490ca60821` |
| kern/trap/trapentry.S | `3a2672c5115d3f0f98139c22a3849179b3939aada8d85bccd63e9b1ea71ff994` |
| tools/grade.sh | `e835d1f1dc211c8de307fd051ddb75441631cb8b0fe31b96447e816ff8cacb97` |

完整修改定位：迁移差异见[migration.diff](evidence/2026-11-10-migration/migration.diff)，模块一差异见[implementation.diff](evidence/2026-11-10-page-mapping/run-4wdontj_/implementation.diff)。本轮仅撰写报告、交接及文档核对清单，未重新生成生产差异。

## 2. 迁移来源与兼容处理

| 来源 | 目标 | 必须保留的兼容点 |
|---|---|---|
| Lab2/3的page_init | `kern/mm/pmm.c`的lab2位置 | 使用目标Page及sizeof，初始化pra_page_link/pra_vaddr，按目标地址宏计算上界 |
| Lab2/3完整Best-Fit | `kern/mm/default_pmm.c` | 包内不存在best_fit_pmm.h，使用default_pmm.h；对象保留default_pmm_manager，测试原样保留 |
| Lab3本人Trap模块 | `kern/trap/trapentry.S` | 以Lab4 SAVE_ALL为准；保留当前帧布局、forkrets及内核S态sscratch约定 |
| Lab3已有handler（含杨昊冉贡献） | `kern/trap/trap.c` | 保留来源；不带入旧实验SHUTDOWN_AFTER策略，保留Lab4 SUM和DEBUG_GRADE |

来源完整摘要已保存在[reference-sha256.json](evidence/2026-11-10-migration/reference-sha256.json)，包括Lab2/3的pmm、Best-Fit和Lab3 Trap文件。目标保留的Lab4结构体、kmalloc、VMA、SAVE_ALL、forkrets、既有评分测试和中断包装，分别有[迁移保留检查](evidence/2026-11-10-migration/preservation.json)及[模块一核对](evidence/2026-11-10-page-mapping/run-4wdontj_/verification.json)。

## 3. 管理器接入和内存接口

`init_pmm_manager()`选择`&default_pmm_manager`；该对象实际绑定best_fit_init/init_memmap/alloc_pages/free_pages/nr_free_pages与原best_fit_check，`.name`为`best_fit_pmm_manager`。启动已实际打印这个名称。Best-Fit是本组集成选择，文件或对象名称不是算法判断依据，不需要再引入Buddy/SLUB开关。

| 接口/状态 | 提供的行为 | 双人组使用时需注意 |
|---|---|---|
| alloc_pages(n)/alloc_page | 连续物理页，失败NULL；管理器n=0沿用断言 | 检查失败；新页ref=0不表示空闲，需要调用者持有与释放 |
| free_pages(base,n)/free_page | 交回空闲管理器并合并 | 必须已无有效使用，不会自动查找并撤销全部映射 |
| nr_free_pages | 当前空闲页数 | 统计页数，不是块数，可用于失败回滚/资源恢复检查 |
| 上述三个包装 | 保存并恢复原SIE | 不要绕过它们；局部保护不等于整个页表/调度事务原子化 |
| pages/npage/nbase | pages[i]描述物理页号nbase+i；数组长度npage-nbase | npage为管理上界页号，不可直接当数组元素数 |
| page2kva/kva2page、PADDR/KADDR | 物理页及内核地址转换 | 使用目标宏，不能把内核虚拟地址直接写成satp页号 |
| boot_pgdir_va / boot_pgdir_pa | 根表内核指针/物理地址 | proc->pgdir应按题目存物理地址；Sv39 satp用MODE与PPN组合，不能用遗留Sv32 lsatp |
| page_insert | 建立或替换普通路径的数据页映射，失败-E_NO_MEM | 同页净引用不变，perm按请求替换；先满足本报告的输入前提 |
| page_remove | 不创建页表的幂等删除 | 最后引用才释放数据页；中间表保留，不得再重复free数据页 |
| kmalloc/kfree、VMA | 原Lab4接口和实现保留 | 未在正式完整初始化链运行；不能仅凭保留源码宣称其到进程管理链已经通过 |

`page_init`先校验范围、溢出、内核覆盖和描述符空间，建立偏移后才用PADDR(end)，保留占用页，仅注册尾部完整页面。默认128MiB的独立镜像中Page为64字节，描述符32768个，占2MiB；具体内核末尾、free_begin和空闲页数随镜像变化，不可照搬独立镜像地址。

### 模块一的资源边界

`get_pte(create=1)`可能分配下级表。第二次分配失败只回滚本次新建的上级表，恢复原父项；原本已有的父表、输入页、旧映射不动。`page_insert`在路径准备成功后才取得新数据页引用；同页不先释放再登记。删除清项并失效后才减引用，降为0才释放；正常删除不收空表，地址空间拥有者要另行清理。

当前只支持有效普通4KiB路径、单hart和外部串行调用，不能沿启动1GiB叶子用get_pte继续向下走，也不能假设损坏表会得到安全-E_NO_MEM。perm=0/W-only是软件契约测试，不是硬件可访问叶子。

## 4. Trap帧、forkrets与新线程首次返回

### 4.1 A提供的实际接口

当前`struct trapframe`是36个8字节槽，共288字节，保持16字节对齐的栈输入经扣除288后仍对齐。

| 内容 | 字节偏移 | 使用约定 |
|---|---:|---|
| x0槽 | 0 | 原SAVE_ALL不初始化；不能据此断言保存槽为0，真实x0始终0 |
| ra/x1 | 8 | 保存/恢复原ra，不能把call trap后的ra当原现场 |
| 原sp/x2 | 16 | 最后恢复，帧尚未读完不得破坏当前sp基准 |
| 其他GPR | 寄存器编号×8 | 布局按pushregs；s0/s1也会在恢复CSR临时使用后恢复 |
| status | 256 | 恢复到sstatus；返回状态须满足当前S态恢复前提 |
| epc | 264 | 写入sepc，接受C层合法修改 |
| badvaddr、cause | 272、280 | 诊断信息，不用于机械恢复CSR |

`__alltraps → SAVE_ALL → a0=帧sp → trap(tf) → __trapret → RESTORE_ALL → sret`已经独立测试。C handler必须遵守ABI和栈平衡，不释放/移走当前仍需恢复的帧，不擅自换到不存在的新帧。合法GPR/epc修改通过帧表达，不能靠改C函数返回值替代修改tf。RESTORE_ALL恢复全部所需GPR并最后恢复sp，sscratch保持当前内核约定0。

本阶段恢复前提为有效内核栈和S态帧。常规进入时SIE已关闭，返回特权级由SPP决定，返回后的中断开关受SPIE控制；不能要求整个sstatus逐位与进入前相同，也不能依赖任意把帧SIE置1后仍安全完成余下恢复。合成帧应保证恢复阶段不提前引入不受控中断。未承诺U态换栈、多hart、任意嵌套或浮点/向量现场。

### 4.2 B应接续的首次运行约定（静态接口说明，尚未整体运行）

现有源码已经有`forkret()`、`forkrets()`和`kern/process/entry.S`的`kernel_thread_entry`，不应被迁移覆盖。首次链路应由双人组构造并验证：

```text
switch_to恢复新context
  → 首次ra指向forkret
  → forkrets(current->tf)
  → sp切为传入的Trap frame地址
  → __trapret，不再执行SAVE_ALL
  → sret按帧epc/status返回
  → kernel_thread_entry：从s1取参数到a0，调用s0中的函数
  → 函数返回后进入do_exit
```

双人组需要保证以下条件，本人没有代写其实现：

- current和current->tf已指向新线程及其有效内核栈中的帧；context.sp为有效、符合ABI的内核栈，context.ra与switch_to返回方式匹配。不能把普通Trap保存与首次合成现场混为一条已执行路径。
- 帧布局与上述288字节一致；gpr.sp是sret后要采用的有效栈位置，不是随意填入堆地址。按题目构造子线程a0返回值、epc及s0/s1入口参数，并核对gp/tp等代码可能依赖的状态。
- 当前内核线程返回S态，应正确设置SPP及SPIE/SIE，不强制回U态，也不在半恢复时提前允许来源未控制的中断。
- 线程共享内核地址空间，页表根须使用物理地址构造Sv39 satp并做必要同步。不能使用libs/riscv.h中的Sv32遗留lsatp。
- 关中断/恢复、PID与链表登记、失败清理属于B的创建/切换责任；A提供的物理页包装不能代替这些操作的整体保护。

`switch_to`对应`struct context`的ra、sp、s0～s11，共14个寄存器；这与Trap保存完整通用现场不同。具体保存偏移和首次栈构造由B实现并提供自己的调试截图。`do_exit()`当前是原包panic，不是完整退出回收；不能悄悄当成已经实现完整生命周期。

## 5. 构建、调试和验证复现

优先使用已有os2026，工具为GCC13.2、QEMU8.2.2、GDB15.1。Makefile默认GDB为gdb-multiarch。下列独立入口均使用当前实际实现，不提供空线程生产函数：

```bash
conda activate os2026
cd /home/lhl/os2026
python3 lab4/tests/migration/run.py
python3 lab4/tests/migration/run.py --gdb-after
python3 lab4/tests/page_mapping/run.py --mode software
python3 lab4/tests/page_mapping/run.py --mode prebuilt
python3 lab4/tests/page_mapping/run.py --mode extended

# 此项目前应失败，退出码1；不要删除失败记录或改条件
python3 lab4/tests/page_mapping/run.py --mode generated
```

完整pmm/Trap初始化链尚未整体通过，本轮整理没有重新运行以上命令。正式`make -j2`的最近一次实际输出在[production-make.log](evidence/2026-11-10-page-mapping/run-4wdontj_/production-make.log)，退出码2，缺proc_init/cpu_idle。没有正式make qemu或make grade通过记录。

### 匹配镜像的GDB演示

终端一运行已验证的software-image并暂停：

```bash
conda activate os2026
qemu-system-riscv64 -machine virt -m 128M -smp 1 -nographic \
  -bios default -monitor none \
  -kernel /home/lhl/os2026/lab4/report/evidence/2026-11-10-demo-visible/software-image/ucore.img \
  -S -gdb tcp:127.0.0.1:37133
```

终端二可按[逐步演示文档](实验四Challenge与GDB演示.md)手动操作；同目录manual.gdb包含匹配ELF、端口及全部读取命令：

```bash
conda activate os2026
gdb-multiarch -q -nx -batch \
  -x /home/lhl/os2026/lab4/report/evidence/2026-11-10-demo-visible/mapping/manual.gdb
```

实际截图执行的是session.gdb，manual.gdb去掉界面握手，未额外重跑裁剪文件。SIE演示用foundation-image和43157端口，完整命令在同一演示文档。端口占用时两端一致换空闲端口，不杀其他实例；断点用当前符号，不能沿用旧ELF地址。GDB的$dbg_*只存主机便利变量，不修改目标内存。

两套本次演示ELF/镜像的SHA256与逐字节配对核对见[verification.json](evidence/2026-11-10-demo-visible/verification.json)。

### 正式集成后的命令（待运行）

```bash
conda activate os2026
cd /home/lhl/os2026/lab4/lab4_new
make
make qemu
# 另起一次调试：终端一make debug，终端二make gdb
make grade
```

这些是下一阶段命令，不是本阶段已成功执行结果。正式ELF必须与启动镜像相配；不要让make gdb加载独立测试ELF或旧bin/kernel。

## 6. 已有证据及其证明边界

| 证据 | 结论 | 不证明的部分 |
|---|---|---|
| [迁移11镜像摘要](evidence/2026-11-10-migration/run-xoikc2sx/summary.json) | startup、foundation O0/O2、context O0/O2、5panic和1无空闲页通过 | 不代表正式初始化全链；context使用受控trap，非法内存输入使用getter替身 |
| [启动前后说明](依赖迁移与验证说明.md) | 原loader/过渡映射问题及修后真实入口 | 不把停机mcause=3当实际取指页异常 |
| [模块一第一批综合结果](evidence/2026-11-10-page-mapping/checks-birpehib/checks.json) | 4通过、2失败；区分自动与规范预置路径 | 不得说综合全PASS |
| [扩展测试与补强说明](tests/page_mapping/独立测试计划与结果.md) | OOM、权限、共享、边界、真实故障等实际结果 | 不覆盖用户态、全部地址或多hart |
| [第007轮extended输出](evidence/2026-11-10-demo-visible/extended.stdout.log) | software/protect/unmap各O0/O2通过 | prebuilt规范父表不能替代生产自动创建 |
| [第007轮GDB映射](evidence/2026-11-10-demo-visible/mapping/gdb.stdout.log) | PTE路径、ref及空闲页的真实前后变化 | 软件项可查不等于硬件可用 |
| [第007轮SIE调试](evidence/2026-11-10-demo-visible/sie/gdb.stdout.log) | 原SIE=0/1均正确恢复；sie=0来源隔离 | 不是多核锁或任意嵌套证明 |
| [自动路径失败](evidence/2026-11-10-demo-visible/generated.stdout.log) | cause=13，VA=0x4000；父项低位V+U | 问题尚未解决 |

A01～A10已在[本人报告](report.md)逐图注明镜像、证明范围和原始日志；图片使用`images/`相对路径。正式全链路、评分和模块二三图片仍待B按各人实际任务补齐，不能挪用A01/A08代替。

真实修复与风险分开记录在报告4.5：启动适配和漏链intr.c已有前后证据；get_pte泄漏属静态发现后修复并验证；memory clobber是约束补强，未证明原先动态重排；非叶子U冲突和原check_boot_pgdir的int循环问题仍保留。没有新增试错故事。

## 7. 同伴接续与最终回归清单

1. 合入前核对本说明8文件摘要和既有测试，保留每位实际负责人学号及迁移来源，不把后续B的实现改署A。
2. 在自己的proc/sched/switch位置完成模块二、三，保持上下文、首次栈与Trap帧的约定一致，处理分配失败和清理，不向A的独立镜像添加假生产功能。
3. 将非叶子U位/课程断言冲突作为独立未解决问题处理：明确兼容策略，保存差异和新证据；不能只清位或改断言后说旧条件全部满足。发现A范围错误交由A核对，不用同伴线程代码掩盖。
4. 正式make链接通过后，从真实kern_init启动，回归pmm_init、kmalloc、VMA、proc_init及clock/idle，特别是之前失败的自动路径和check_boot_pgdir。
5. 演示idleproc到initproc的首次返回、两个测试线程获得执行、切换后恢复、current/页表一致、局部中断保护原状态，以及资源失败路径；提供B各自真实日志和截图。
6. 正式make grade核对三组标记`[Test PMM] PASS`、`[Test Process Create] PASS`、`[Test Schedule] PASS`；当前脚本每组10分、合计30分，还检查initproc输出。保留DEBUG_GRADE实际行为，不沿用旧Lab3关机阈值或分数口径。
7. 即使三组标记齐全，仍记录边界覆盖；不得把旧check_boot_pgdir打印成功当作int循环已遍历全部高地址映射。
8. 三人分别补本人报告、AI工具/模型、提示词和截图，填写小组最终日期，合并相对路径。当前没有同伴正文，后续已有正文应保留，不由A代写。
9. 仓库目标仍是后续明确授权后lab4分支的code/report结构；本阶段不重排代码。届时若导出到code，应核对源码链接，保留证据出处，不自动commit/push。

## 8. 交付文件与确实缺少的材料

| 本人已交付 | 位置 |
|---|---|
| 迁移、模块一、Challenge及本人阶段报告 | [report.md](report.md) |
| 交接接口与版本定位 | 本文件 |
| 模板/题目/链接/事实核对 | [成员A文档核对清单.md](成员A文档核对清单.md) |
| 已存在的真实对话记录 | [prompt.md](prompt.md)，不把规划指南并入记录 |
| 本人有效图片 | images/A01～A10，共10张，逐图引用已整理 |
| 实现、独立支架与基线 | ../lab4_new、../tests、../baseline，均保持现状 |

真正仍缺的是：自动建表兼容性问题的解决与回归、模块二/三正式实现及本人报告图片、正式构建/启动/评分结果、完整首次运行和调度演示、本实验三人实际AI模型信息及小组最终日期。本人尚无正式全链路截图，未将此缺项隐藏在独立测试图片中。本轮没有重跑内核，也没有提交Git。
