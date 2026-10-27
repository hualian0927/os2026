# 实验三成员A阶段交接说明

成员A：李华濂，学号2412351。接收成员：肖子昂、杨昊冉。阶段版本：A1，2026-10-27。

本阶段我完成了实验二内存初始化与Best-Fit的迁移、当前环境所需的启动和调试适配，以及功能模块一的Trap入口、上下文恢复和独立验证。模块二的`interrupt_handler()`、`exception_handler()`尚未实现，完整构建仍受这两个函数缺失影响。我没有将独立镜像通过写成整个实验通过。

本文提供本人负责部分的代码说明、已有验证证据和接入约定，不代替最终`report.md`。肖子昂、杨昊冉继续负责模块二、完整集成、最终报告排版、图片和提示词汇总；本次交接不新增实际提示词记录。

## 一、交付位置与版本

本机开发目录仍为`/home/lhl/os2026/lab3/lab3_new`，没有重排。GitHub的`lab3`分支采用与前两次实验相同的顶层`code/`、`report/`结构：`code/`对应当前正式源码，`report/`保存本文、独立验证材料、原始日志及图片占位。阶段提交不创建最终`report.md`或`prompt.md`，也不将提示词指南作为交付物。

以下六个文件是相对`lab3.zip`初始代码的全部正式改动，版本用当前内容SHA-256前12位定位，完整清单和差异见[阶段清单](evidence/stage-manifest.json)与[源码差异](evidence/source-diff.patch)。Git提交号可在交付分支执行`git log -1 --oneline`取得，不把未产生的提交号填入本文。

| 正式文件 | 当前内容版本 | 本人完成内容 |
|---|---|---|
| `kern/mm/pmm.c` | `9141f4ab45ff` | 迁移`page_init()`；接入Best-Fit；保留Lab3公共接口的中断保护 |
| `kern/mm/best_fit_pmm.c` | `6583073dd7a2` | 迁移完整管理器实现，保留目标包已有自测 |
| `kern/init/entry.S` | `42ee40f56825` | 增加分页切换期间需要的低地址恒等映射 |
| `kern/trap/trapentry.S` | `7a752be33258` | 完成两个`lab3 function1`位置，填写2412351 |
| `Makefile` | `9187cb930c17` | 适配QEMU镜像交接、实际GDB工具和共用调试端口 |
| `tools/grade.sh` | `fd190416357b` | 仅将镜像加载参数同步为`-kernel` |

`SAVE_ALL`、`pushregs/trapframe`、`trap()`签名、`trap.c`、`pmm.h`、`memlayout.h`、`riscv.h`和`sync.h`保持初始包内容；`basic_check()`、`best_fit_check()`及题目三个`test_lab3*`文件、`inject.py`保持不变。评分脚本不是整个文件未改，但加载参数以外的内容未改，包括匹配条件、分值与超时。上述关系有清单中的逐项比较记录。

## 二、实验二依赖迁移与兼容性

迁移来源是本人已完成的`/home/lhl/os2026/lab2/code/kern/mm/pmm.c`和`best_fit_pmm.c`，没有整文件覆盖Lab3的`pmm.c`，也没有引入Buddy、SLUB或其他管理器开关。沿用Best-Fit作为默认管理器是本组的集成选择，并非声称题目要求所有组必须选择它。

Lab3没有直接提供实验二使用的两个边界宏，因此我用目标框架已有定义计算：

```c
memory_limit    = KERNTOP - PHYSICAL_MEMORY_OFFSET;
kernel_begin_pa = KERNBASE - PHYSICAL_MEMORY_OFFSET;
```

其余部分使用Lab3现有`Page`、`pmm_manager`、设备树getter、地址转换宏和链接符号`end`。`end`由`tools/kernel.ld`在BSS之后定义；不能根据源码文件大小猜测内核结束地址。页描述符状态操作采用目标框架已有宏，不把实验二头文件复制过来覆盖Lab3接口。

### 1. page_init的结果与边界

设备树已经初始化、启动映射及内核栈有效时，`page_init()`先校验内存信息，再设置`va_pa_offset`，之后才使用`PADDR(end)`。描述符数组安排在内核尾部向上按页对齐的位置，先验证容量再写入，数量按`npage-nbase`计算，大小使用`sizeof(struct Page)`。

`npage`是可管理物理页号的排他上界，不是数组元素数，也不是空闲页数。默认128 MiB内存下，物理范围为`[0x80000000, 0x88000000)`，有`nbase=0x80000`、`npage=0x88000`，描述符数量为32768。实际空闲页还需扣除固件、内核、描述符等保留页。

初始化先将所有描述符设为保留，只将描述符尾部之后的完整页面交给管理器。零基址或长度、范围溢出、内核不在有效范围及描述符空间不足会panic；没有完整空闲页时不调用注册接口。这仅说明`page_init()`的边界行为，后续分配器自测仍需要足够的空闲页，不能据此保证零空闲页时整个`pmm_init()`也能成功。

### 2. Best-Fit接口与不变量

`best_fit_pmm_manager`完整绑定初始化、区域注册、分配、释放、页数查询和既有检查函数。空闲链表仅挂块头，按物理地址升序排列；块头`property`表示连续页数，`PageProperty`标识块头，`nr_free`统计页数。

分配遍历全部候选块，选最小的足够大块；相同大小保留先找到的低地址块。分割后把剩余块放回原顺序位置，成功只扣申请页数；失败返回NULL且不改链表和计数。释放时按地址插入，先合并物理连续的前块，更新`base`后再检查后块。`n=0`沿用断言契约，调用者不得重复释放或释放保留区域。

### 3. Lab3局部中断保护

我保留了目标代码已有的`alloc_pages()`、`free_pages()`、`nr_free_pages()`包装，没有用实验二版本覆盖它们。三者均先保存当前SIE状态，关闭本hart的中断后调用管理器，最后按原状态恢复；不是函数结束就无条件开中断。

在原SIE为0时返回后仍为0，原SIE为1时临界区内部为0、返回后恢复1。独立探针和实际基础镜像均检查了这两种状态；动态SIE=1检查前将`sie`来源清零。它证明的是当前单hart局部保护，不是多hart互斥或中断竞争压力测试。正式启动的`pmm_init()`在时钟初始化和全局开中断之前执行，初始区域注册仍处于受控启动阶段。

## 三、功能模块一原理与接口约定

实际调用链为：

```text
硬件进入S态Trap，写入sepc/scause等CSR
→ __alltraps / SAVE_ALL
→ a0=当前帧sp，call trap(tf)
→ trap_dispatch(tf) → 模块二handler
→ C函数正常返回至__trapret
→ RESTORE_ALL → sret
```

硬件不自动把32个通用寄存器压入内存；这部分由`SAVE_ALL`完成。RV64下每槽8字节，结构体共36槽、288字节。原sp通过`sscratch`暂存后写入第2槽；入口最终将`sscratch`清零，沿用当前内核S态约定。

| 内容 | 相对帧首地址的字节偏移 |
|---|---:|
| x0～x31 | 寄存器编号×8，其中原sp为16 |
| status | 256 |
| epc | 264 |
| badvaddr | 272 |
| cause | 280 |

若进入前`sp=S`，保存后`sp=F=S-288`，则传给C的是`tf=F`，帧内`tf->gpr.sp=S`。不能将原sp或某个字段地址当成参数。288是16的倍数，原栈按ABI对齐时，调用C仍满足对齐要求。

我在恢复时先用`s0/s1`加载处理后的`status/epc`并写回`sstatus/sepc`，随后恢复通用寄存器，最后从第2槽恢复sp。这样CSR操作不会再次覆盖已经恢复的通用寄存器，帧读取期间寻址基准也不会提前改变。x0保持硬连零；`cause/badvaddr`用于诊断，不机械写回。传参改写a0和`call`改写ra之前，原值已在帧内保存；返回时从帧恢复。

### 模块二应遵守的约定

1. 接收实际有效的`struct trapframe *tf`，不改变结构体布局；帧所在栈和返回目标必须可访问，不能破坏帧或让C层遗留错误sp。
2. 处理后正常返回，由汇编完成恢复和`sret`。C层遵守RV64调用约定；需要改变被中断现场时修改帧，而不是依赖C的临时寄存器值。
3. 允许修改帧内的合法通用寄存器值和`epc`。恢复逻辑服从处理后的帧，不使用入口旧值覆盖；直接改硬件`sepc`却不改`tf->epc`，会被恢复过程中的帧值覆盖。
4. 同步异常需要结合实际编码判断后继地址，不能把所有异常都统一加4；压缩指令可能为2字节。时钟中断通常应返回原被中断位置，不按异常指令长度推进epc。
5. 当前场景保持`SPP=1`返回S态。恢复阶段帧内`SIE`应保持0，返回后是否开中断通过`SPIE`表达。`sret`令`SIE=SPIE`、`SPIE=1`、`SPP=0`，因此不能要求返回后的整个`sstatus`等于保存值。
6. `cause/badvaddr`可以用于分类和打印，其帧内修改不是改变实际硬件异常原因的协议。`sscratch=0`也不代表已经实现用户栈切换或任意嵌套支持。

上述返回前提不包含损坏栈、无效目标地址、任意嵌套、多hart、用户态换栈及浮点/向量现场管理。

## 四、真实问题、处理和理论风险

| 项目 | 已有事实与证据 | 当前处理 |
|---|---|---|
| 固件未交接正确入口 | 同一隔离镜像用loader参数时OpenSBI显示`Next Address: 0x0`；改用`-kernel`后显示`0x80200000` | 修改Makefile的qemu/debug/test以及评分启动器加载参数；不改评分判据 |
| 分页切换取指页错误 | 原根表只有高地址映射，日志出现cause=`0xc`、epc/tval=`0x80200038` | 增加根项2的低地址恒等映射，保留根项511；没有插nop掩盖错误 |
| 调试器名称不匹配 | 当前环境没有`riscv64-unknown-elf-gdb`，已有支持RV64的`gdb-multiarch` | Makefile默认使用实际工具，并允许覆盖GDB、GDBPORT |
| 模块二缺失 | 正式构建日志报两个handler隐式声明错误 | 这是尚未完成的依赖，不补生产空函数、不删除调用 |
| 原测试驱动ABI问题 | `test_lab3_context.S`直接改写s0～s11后ret，未保护普通调用者；C文件按普通函数调用它 | 源码审查确认，未动态复现评分故障；保持题目测试不改，独立驱动保护自身ABI，建议向助教说明 |

前三项是已经发生并有日志的环境/启动问题，均已处理。原测试驱动问题是静态发现，不能写成“实际评分因此失败”。若调用者把有效变量保存在s2中，该辅助函数返回的s2却是测试哨兵，就会违反ABI；是否影响某次评分还取决于实际调用代码。

另一个理论风险是：handler若将帧内SIE设为1且存在已使能的挂起中断，恢复未完成时可能再次进入Trap。这不符合当前接入约定，本阶段没有据此扩展嵌套实现。

负面对照中人为删掉`sepc`写回、删掉`s0`恢复，是为了验证测试能检出错误，**不是正式代码曾经出现并被修好的Bug**。

## 五、实际验证与证据范围

| 验证项目 | 已有结果 | 覆盖局限 |
|---|---|---|
| 内存初始化 | 13个隔离用例通过，含零范围、溢出、空间不足和无完整空闲页 | 用例提供DTB getter结果及分配器回调，不等于真实DTB全链路 |
| Best-Fit | O0/O2通过既有检查、边界检查和各512步位图核对 | 启动、UART及DTB getter为测试设施，非正式完整启动 |
| 真实基础镜像 | 实际entry、DTB、SBI、PMM及Best-Fit通过；保留页与回收后状态正确 | 使用专用kern_init和意外Trap停止点，不运行正式模块二 |
| 局部中断包装 | 原SIE为0/1均通过，探针确认内部为0 | 事先关闭所有Supervisor中断来源，无异步竞争压力 |
| GDB与启动参数 | 入口a0=0、a1=0x87e00000，DTB魔数正确；进入高地址C；根项2/511均为0x200000cf | 地址属于当次隔离ELF，不能固定套到最终ELF |
| 模块一首轮检查 | O0/O2各8次真实Trap通过，汇编目标文件编译成功 | 测试trap替身，不是正式handler |
| 模块一独立检查 | O0/O2各64次真实Trap通过，覆盖寄存器、真实C调用、epc与a0/s0/s1/t0修改、SP稳定、ABI及状态转换 | 仅受控32位断点，无异步时钟、压缩断点或正式分发链 |
| 测试敏感性 | 两个错误汇编副本均产生明确失败标记，无超时 | 两项针对性对照，不是穷尽所有可能错误 |
| 正式完整构建与评分 | 既有构建失败日志仍由两个handler缺失解释；未取得正式评分通过结果 | 待双人组接入后回归 |

最新独立测试链接的是正式`trapentry.S`，没有重写另一套Trap保存恢复来测试自己；驱动自己的普通函数序言只保护调用者ABI，返回快照在序言恢复前取得。QEMU提供真实DTB，但模块一测试不解析它，也不提供假DTB getter；`gp/tp`保留真实值，没有主动破坏，因此不能宣称动态检查能识别其所有漏恢复错误。

本次整理交接时只比较当前源码、已有快照与原始输出，没有重新运行上述测试，也没有新增截图。最新64次检查的正式目录哈希与当前对应文件一致。证据按以下位置保存：

- [基础验证日志索引](evidence/README.md)：迁移、启动、GDB、边界回归和构建阻塞。
- [模块一测试说明](tests/module1/README.md)、[脚本原始输出](tests/module1/results/run-stt3fs9o/output.txt)、[结构化结果](tests/module1/results/run-stt3fs9o/summary.json)。
- [O0原始串口](tests/module1/results/run-stt3fs9o/O0/console.log)、[O2原始串口](tests/module1/results/run-stt3fs9o/O2/console.log)。

既有日志中保留当时的绝对路径，未将历史路径替换成新路径冒充新运行。交付只保存文本证据和测试源码，不上传可执行镜像、对象文件或压缩包。

## 六、构建、独立验证与调试命令

实际环境复用conda的`os2026`：GCC13.2.0、binutils2.42、QEMU8.2.2、OpenSBI1.3、gdb-multiarch15.1，没有新增安装。当前正式源码因模块二缺失不能完成`make`；以下完整工程命令应在双人组补齐后使用。

```bash
conda activate os2026
# GitHub分支克隆后的根目录
cd /path/to/os2026-lab3
cd code
make clean
make
make qemu
```

双终端调试：两个终端都在同一份`code/`中，终端一运行`make debug`，终端二运行`make gdb`。默认端口1234，加载`bin/kernel`，必须和终端一使用的`bin/ucore.img`来自同次构建。也可在两端一致指定`GDBPORT=15350`；工具可用`GDB=gdb-multiarch`覆盖。`make debug`启动后暂停等连接，GDB中的`continue`才继续执行。

模块二未完成时，可以在分支根目录运行交付的独立模块一入口：

```bash
conda activate os2026
cd /path/to/os2026-lab3
python3 report/tests/module1/run.py --source "$PWD/code" --negative-controls
```

脚本用正式源码重新构建独立镜像，结果生成在`report/tests/module1/results/run-*`。必须传`--source`：原测试脚本保持开发目录版默认规则，没有为打包改写测试源码。每个QEMU有10秒超时，仅清理自己启动的进程。失败替身通过SBI关机也可能返回退出码0，脚本会检查失败标记，不能只看QEMU退出码。

如需复现基础启动和内存管理隔离镜像，可使用一并保存的原测试入口：

```bash
cd /path/to/os2026-lab3
python3 report/tests/foundation/build_fixture.py "$PWD/code" \
    /tmp/os2026-lab3-A-foundation
qemu-system-riscv64 -machine virt -nographic -bios default \
    -kernel /tmp/os2026-lab3-A-foundation/ucore.img -monitor none
```

手动调试该镜像时，在上述QEMU命令末尾添加`-gdb tcp:127.0.0.1:15350 -S`，另一终端运行：

```bash
gdb-multiarch -q /tmp/os2026-lab3-A-foundation/kernel \
    -ex 'set arch riscv:rv64' \
    -ex 'target remote localhost:15350'
```

可查看的命令如下；这里给出的是重现操作，历史实际观测见GDB原始日志，不表示本轮又执行过：

```gdb
set pagination off
hbreak *0x80200000
continue
info registers pc a0 a1 satp
x/4bx $a1
hbreak *kern_init
continue
info registers pc sp satp
p/x &bootstacktop
x/gx ((char *)&boot_page_table_sv39)+2*8
x/gx ((char *)&boot_page_table_sv39)+511*8
hbreak pmm_init
continue
finish
p/x npage
p/x nbase
p/x pages
detach
quit
```

`*kern_init`用于在函数第一条指令处观察栈；按源码行断点可能停在序言之后，不能直接要求当时sp仍等于bootstacktop。隔离镜像不含正式时钟处理；不要用它说明最终中断任务已经完成。13个内存边界及Best-Fit回归脚本曾在明确的临时副本中执行，本文保存原输出，没有把Lab2脚本直接塞入正式Lab3并宣称开箱可运行。

## 七、建议截图位置

本阶段没有新增测试截图；以下是双人组汇总时可取得的画面，统一标注镜像、版本及检查范围，放入`report/images/`。日志画面应注明是既有日志，不能当作当场重新执行。

| 建议图片名 | 建议画面 | 已有依据或取得方式 |
|---|---|---|
| `A01-foundation.png` | Best-Fit名称、真实DTB范围、基础检查PASS | 基础隔离镜像运行或查看`after-foundation.log` |
| `A02-boot-gdb.png` | a0/a1、分页后的PC/SP、根项2和511 | 按上述双终端命令重现；对照`after-paging-gdb.log` |
| `A03-memory-regression.png` | 13个page_init用例、O0/O2 Best-Fit回归 | 已存`page-init.log`、`best-fit.log`，注明历史独立检查 |
| `A04-module1.png` | O0/O2各64次Trap及各检查PASS | 运行交付的模块一脚本；原输出已保存 |
| `A05-negative-controls.png` | 两个故意错误副本被检出、正式目录未改 | 同一脚本的负面对照输出，不能写成正式实现失败 |
| `A06-frame-debug.png` | 实际tf指针、epc、原sp及返回位置 | 建议在重建的模块一隔离ELF上断到trap；这是待补演示，不是已有截图 |

完整内核启动、压缩断点、非法指令返回、持续时钟和正常关机的最终截图仍需双人组在完成模块二后取得。不能将题目中的scause插图当成测试截图，也不能仅凭打印字符串替代状态检查。

## 八、下游接入及最终演示前的回归清单

双人组接入点是`kern/trap/trap.c`中`lab3 function2 YOUR CODE`区域，按实际负责人填写学号；`trap()`和`trap_dispatch()`已有调用链，不需要重写模块一。仍需完成异常分类、断点/非法指令输出与epc更新、未知情况诊断、时钟重设与计数、周期打印和正常关机。

题目提到`SHUTDOWN_AFTER`，但当前包没有给出定义或确定数值，需核实课程要求或明确本组约定，不能猜一个值写成题目规定。既有持续时钟测试要观测到`ticks=200`，关机不能先于必要检查结束；有冲突应核实，不改评分判据。

- [ ] 确认所有本人提交文件与阶段清单一致，模块二只接入本人职责范围外的规定位置。
- [ ] 清理后构建正式工程，确认Best-Fit输出、真实DTB、内存自测和Trap初始化顺序。
- [ ] 正式handler参与后复查模块一返回：寄存器、sp、epc修改与状态位关系。
- [ ] 检查2字节压缩断点、4字节指令、非法指令返回及兜底诊断；不要全部固定加4。
- [ ] 持续时钟多次到达，每次重新设置后继事件且ticks仅加一次，输出周期正确。
- [ ] 核实关机阈值并验证正常关机，不把独立测试结束调用SBI关机算成阈值逻辑通过。
- [ ] 执行正式`make grade`，记录分项与原始输出；保留题目测试，注意现有测试驱动的ABI问题。
- [ ] 在同一个最终ELF/镜像上复核`make debug`、`make gdb`和现场退出方式。
- [ ] 肖子昂能够演示双人组负责的全部模块二；本人能够解释迁移、帧布局及恢复路径。
- [ ] 双人组完成最终报告、图片、图注和后续明确要求的提示词材料，校验结论与真实输出一致。

若接入后发现本人迁移或模块一问题，由我针对证据修复；模块二实现和报告最终整理仍由双人组负责。阶段提交只表示本人的前段工作已交付，不表示实验三已经全部完成。
