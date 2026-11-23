# Lab5成员A阶段交接说明

李华濂，2412351；2026-11-23。交接对象：肖子昂、杨昊冉。

我已完成前序必要迁移、双向Trap和模块一分发及适配代码，并对当前版本重新进行了独立验证。本阶段可以交接接口和本人证据，尚不能作为完整Lab5验收版本。真实用户进程装载、复制、等待、退出和调度运行仍需后续模块；`sys_pgdir`真实输出还缺框架函数`print_pgdir`的定义。

本轮在保留失败证据后，仅修改生产trap.c的时钟重新调度通知，没有改题目检查或同伴占位；另补充独立测试中的真实U态拒绝和时钟自旋用例及本交接材料。未追加对话记录，未commit或push。

## 一、交接版本与改动范围

原开发工程：`/home/lhl/os2026/lab5/lab5_new`；本分支交付工程为根目录`code/`。原包保存在report/lab5.zip，SHA256为：

```text
4dd863d539fc70eb6df24eaab2b45cda7cbf9c7d47817fbee20bda4926b42ae2
```

本阶段版本以96个生产文件的相对路径和SHA256清单标识，不使用不存在的提交号。清单排序后序列化所得摘要为：

```text
c430c8abf12d45dde02320b90ac644b8e5e5e1e6fc8e2920cccee913cc1ee504
```

原包到当前版本共11个文件有差异。下表摘要为各文件SHA256前12位，完整值见[源码清单](evidence/2026-11-23-audit/source-after.json)，完整差异见[原包到交接版本的diff](evidence/2026-11-23-audit/migration-and-module-one.diff)。

| 文件 | 摘要 | 已完成内容 |
|---|---|---|
| Makefile | 2a0640699710 | 正式qemu/debug使用-kernel，检测现有RV64 GDB |
| kern/init/entry.S | be194d1f3b3a | 保留hartid/DTB传递，补齐符号低位和分页切换时的低地址映射 |
| kern/mm/pmm.c | de7128cc7918 | page_init、页映射迁移、get_pte新增中间表分配失败回滚；保留目标中断保护 |
| kern/mm/default_pmm.c | 79a55f306ee7 | 完整Best-Fit，适配目标default_pmm_manager对象与原自测 |
| kern/process/proc.c | fceb73f5ea4d | 迁移copy_thread/proc_init/cpu_idle/proc_run；Lab5新增创建和生命周期占位仍保留 |
| kern/process/switch.S | 86a70e2b9c19 | 迁移原上下文切换，保留原实现者2312115来源 |
| kern/schedule/sched.c | 335efb8d470a | 迁移原基础调度，保留来源和目标wakeup_proc |
| kern/trap/trapentry.S | d1323bf70199 | 匹配目标SAVE_ALL的RESTORE_ALL/__alltraps，保留forkrets |
| kern/trap/trap.c | 7f6415a7d06f | 用户ecall及异常返回；本轮补齐时钟的need_resched通知 |
| kern/syscall/syscall.c | a74f44f8f95a | 全部9个sys_*及分发、边界、用户范围检查和返回适配 |
| user/libs/syscall.c | c07b098d8923 | 保持公开签名，修复内部宽度、可变参数和内联汇编ABI |

迁移来源是已完成的Lab4工程及其Lab2/3基础，不是用旧文件整文件覆盖。Best-Fit作为默认算法是本组集成选择，实际对象仍为目标框架的`default_pmm_manager`，输出为`default_pmm_manager (Best-Fit)`；实际回调按最小足够块选择，同大小选低地址。

本轮核对了原包中的test_lab5.c/.h、inject.py、grade.sh、trap.h、sync.h、proc.h和SAVE_ALL均保持不变，记录见[保留检查](evidence/2026-11-23-audit/preservation-before.json)。本轮前后只有trap.c的通知修复不同，见[最小修复](evidence/2026-11-23-audit/timer-fix.diff)；原输入版本及全部日志保留。

## 二、审查结论与证据范围

| 规格与审查位置 | 结论及可能失败条件 | 当前证据 |
|---|---|---|
| entry.S：a0/a1、根表、栈及高地址跳转 | 在当前单hart QEMU virt/OpenSBI条件下可进入高地址C；不推广到任意启动协议 | 本轮真实DTB与匹配ELF的GDB启动检查通过 |
| page_init：偏移、页数与描述符 | 默认128MiB下npage=0x88000、nbase=0x80000，描述符数32768；内核与描述符页保留，可用区按整页注册 | 本轮-O0/-O2检查；五种错误输入隔离panic，第六种无完整空闲页不注册 |
| Best-Fit及分配器接口 | 最小足够块、低地址平局、分割合并、页数恢复符合约定；n=0仍为断言前提 | 原分配器自测及独立选择用例通过 |
| 中断保护 | alloc/free/count保留原来的local_intr_save/restore；每层独立标志、逆序恢复；不等于SMP锁 | 受控sie来源下，原SIE为0/1和嵌套用例通过 |
| get_pte失败回滚 | 第二级分配失败时只回收本次新建的上级表，已有表不回收 | 实际pmm实现的失败注入用例通过 |
| 页表实际硬件访问 | 原非叶PTE_U与原断言冲突仍存在；不能用软件PTE检查通过推断生产访存成功 | 本轮真实写再次触发cause=15、tval=0x100；未修改断言或权限规则 |
| SAVE_ALL/RESTORE_ALL | 288字节帧、原sp、通用寄存器、C改帧与CSR临时寄存器恢复符合当前受控场景 | 实际trapentry.S在-O0/-O2下分别完成64次S态和64次U态Trap检查 |
| trap/current->tf | C入口收到本次帧；trap暂存旧tf并在正常返回时恢复，内容修改仍保留 | 实际trap与软件帧接口检查、真实U态后端观察通过 |
| 时钟与用户自旋 | 原迁移未发出重新调度通知，无法让用户自旋流让出CPU；本轮补齐 | 修复前真实时钟100 ticks后仍超时；修复后tick 1到达调度观察入口 |
| ecall/EPC/分发表 | 仅用户ecall接入模块一，EPC先加4；完整64位编号、空项和返回符号扩展符合约定 | 原Syscall断言、软件接口检查及本轮真实U态拒绝用例通过 |
| 参数与用户范围 | 五个RV64参数按序，PID等有符号int范围检查；NULL等待合法，其他指针按范围及VMA权限检查 | 软件边界/栈保护检查及真实U态只读/空洞/内核地址/跨界/溢出拒绝通过 |
| getpid/putc | getpid读取实际current->pid；putc调用实际控制台路径 | 真实U态公开封装返回正确，16个字符由原控制台实际输出 |
| fork/exec/wait/exit/kill等 | 适配层的输入输出可交接，业务语义尚未证明 | do_*只存在于独立镜像的观察探针中，不能视为进程业务通过 |
| 基础创建、切换和调度迁移 | 对象编译通过；尚未在真实进程生命周期中动态验收 | 本轮编译有alloc_proc/init_main未定义告警；没有掩盖为完整链接成功 |
| sys_pgdir | 调用接口接通，真实打印未完成 | 测试仅观察调用；正式链接缺print_pgdir定义 |

本轮发现并复现了一项本人迁移错误：沿用Lab4时钟分支后，只重装定时器和计数，未设置need_resched。原user/spin.c的子进程一直自旋，父进程必须在之后重新获得CPU才能执行kill，因此缺少该通知会阻断后续生命周期测试。

先运行相同的真实U态自旋/硬件时钟镜像，实际输出100 ticks后仍未到达schedule观察函数，2秒超时，见[修复前console](evidence/2026-11-23-module-one/run-1vgg8sxw/timer-O2/console.log)和[失败摘要](evidence/2026-11-23-module-one/run-1vgg8sxw/summary.json)。随后只在时钟分支增加current非空时设置need_resched；由既有trap出口仅对U态返回执行调度，不在中断处理函数里调用schedule，不更改调度器或同伴进程业务。

每次已有时钟tick通知一次是本阶段的集成选择，不把题目未规定的调度时间片当成评分要求，也不将通知频率绑定到原100 ticks打印阈值。相同测试修复后在第1次tick到达观察入口，见[修复后console](evidence/2026-11-23-module-one/run-qh0n7lno/timer-O2/console.log)。观察函数只选择受控返回，不代表真实父子进程切换、kill或wait通过。此前用户封装的静态错误及反汇编见[模块一说明](模块一实现与验证说明.md)，没有虚构旧用户封装崩溃。

## 三、Trap和返回接口

目标帧由32个通用寄存器槽和4个CSR槽组成，每槽8字节，共288字节。x2槽偏移16，status/epc/tval/cause偏移分别为256/264/272/280；288满足16字节栈对齐。

U态进入时，SAVE_ALL交换sp与sscratch，使用可信内核栈保存现场，将用户sp写入帧的x2槽，然后清零sscratch。S态进入时sscratch为0，继续使用原内核栈。__alltraps以当前帧sp作为a0调用真正的trap(tf)，传递的不是用户sp。

RESTORE_ALL先采用帧内status和epc写回控制CSR，再恢复通用寄存器，最后读取x2槽恢复sp。CSR临时使用的s0/s1/s2随后也恢复，读取帧时不提前破坏寻址基准。tval/cause用于诊断，不机械写回。返回S态时sscratch为0，返回U态时为本次帧顶端加288，供下次Trap使用。

sret按SPP决定返回级别，以SPIE恢复SIE，并按架构更新SPIE/SPP，不能要求整个sstatus逐位不变。当前可信帧在恢复期间应保持SIE关闭，返回后的中断状态通过SPIE表达。本阶段不承诺任意嵌套、多hart或损坏栈的容错。

给后续模块的要求：保持当前内核栈和帧有效，修改有效帧内容，不把current->tf换成失效的局部对象；成功exec应更新入口、用户sp和返回状态，不能在调用返回后再次统一推进EPC。fork子帧应独立存放，a0为0，入口继承已推进的EPC。

forkrets本身没有改变：a0必须是新进程有效的trapframe地址，它令sp指向该帧并跳入__trapret。copy_thread已将context.ra设置为forkret、context.sp设置为子帧；肖子昂的kernel_thread和杨昊冉的do_fork须与这一约定一致，不能把用户栈当作可信内核栈。

## 四、系统调用与后端交接表

用户a0为调用号，a1～a5依次为参数；a0承载有符号返回值。支持编号为1/2/3/4/10/12/18/30/31，其他编号即使在unistd.h中有名称也不表示已经接入。整数窄化前检查范围，指针和长度保持RV64宽度。非法号、空项和入口参数错误返回-E_INVAL；后端其他错误原样传播。

| 本人入口 | 后端输入 | 负责人及返回/资源约定 |
|---|---|---|
| sys_exit | do_exit(int code) | 杨昊冉；成功不返回用户，僵尸态、mm回收和通知由后端完成 |
| sys_fork | do_fork(0, tf->gpr.sp, tf) | 杨昊冉；普通复制，父返回PID，子帧a0=0，失败资源回滚由后端完成 |
| sys_wait | do_wait(int pid, int *store) | 肖子昂；pid=0和store=NULL有实际用户封装；可能阻塞，成功写4字节退出码并回收子进程 |
| sys_exec | do_execve(name, len, binary, size)，依次来自a1～a4 | 肖子昂；没有现成用户sys_exec封装；成功不回旧程序后继，失败时旧/新mm资源处理须由后端明确 |
| sys_yield | do_yield() | 杨昊冉；原测试要求返回0并设置need_resched，实际调度由返回链及调度器完成 |
| sys_kill | do_kill(int pid) | 杨昊冉；原测试要求-1返回-E_INVAL；退出标志、唤醒和最终退出由后端完成 |
| sys_getpid | current->pid | 本人；current有效即可，不要求mm非空 |
| sys_putc | cputchar(int c) | 本人；实际输出后返回0 |
| sys_pgdir | print_pgdir() | 本人接口已接通；真实函数定义仍缺，不转嫁为同伴已经完成或本轮已验证 |

参数和资源处理还需遵守：

- 非NULL等待地址必须覆盖4字节可写范围；不能在无效指针错误前先消耗可回收子进程。后端应在实际写回和回收阶段再次确认数据与资源仍有效。坏参数测试还会再次用正确地址等待同一个子进程。
- exec名称长度为0时入口不读取名称字节；后端不能对该未经检查的指针无界strlen。长度非0时入口检查整个名称范围；二进制必须为非空可读范围。ELF解析和命名规则仍由后端决定。
- user_mem_check(mm==NULL)接受的是内核范围，因此sys_*的用户缓冲区检查显式排除此情况。可信kernel_execve按指导书直接调用do_execve并准备异常返回，不能靠伪造U态ecall让内核二进制通过用户指针检查。
- VMA检查不等于页面始终驻留、数据被锁定或异常复制可恢复。copy_from_user/copy_to_user目前是检查后memcpy，后端应明确检查、复制、睡眠和销毁旧mm的顺序。
- 等待与退出可能调度或不返回，不要把整个syscall包在长期关闭中断的临界区里。保留已有局部保护和进程层锁的各自职责。

## 五、本轮真实验证与原始证据

复用现有os2026环境：GCC 13.2.0、QEMU 8.2.2、GDB multiarch 15.1，未安装工具。

```bash
conda activate os2026
cd /home/lhl/os2026-lab5
python3 report/tests/foundation/run.py
python3 report/tests/module_one/run.py
python3 report/tests/module_one/run.py --timer-only
python3 report/tests/foundation/run.py --only conflict
```

| 运行 | 原始日志与镜像 | 实际结论 |
|---|---|---|
| 基础迁移回归 | [run-sa7sq6eh](evidence/2026-11-23-foundation/run-sa7sq6eh)、[summary](evidence/2026-11-23-foundation/run-sa7sq6eh/summary.json) | 修复后13个独立镜像通过，包括启动、两个优化级别的内存/S态/U态及六个隔离初始化边界 |
| 模块一回归 | [run-4kl6dlox](evidence/2026-11-23-module-one/run-4kl6dlox)、[summary](evidence/2026-11-23-module-one/run-4kl6dlox/summary.json) | 修复后4个独立镜像通过：两优化级别的软件接口与真实U态链 |
| 真实时钟自旋 | [run-qh0n7lno](evidence/2026-11-23-module-one/run-qh0n7lno)、[summary](evidence/2026-11-23-module-one/run-qh0n7lno/summary.json) | 修复后通过；实际中断及重新调度请求验证，非真实调度算法测试 |
| 原框架冲突 | [run-kug6w2b2](evidence/2026-11-23-foundation/run-kug6w2b2)、[console](evidence/2026-11-23-foundation/run-kug6w2b2/framework-selftest/console.log) | 命中预期访存冲突，是冲突观测成功，不是原check_boot_pgdir通过 |
| 对象重新构建 | [objects-after-fix.log](evidence/2026-11-23-audit/objects-after-fix.log) | 指定迁移及模块一对象编译成功，保留alloc_proc/init_main缺失告警 |
| 正式完整构建 | [full-build-after-fix.log](evidence/2026-11-23-audit/full-build-after-fix.log) | 链接失败；缺少alloc_proc/init_main/kernel_thread、六个do_*和print_pgdir |
| 原make grade | [grade-after-fix.log](evidence/2026-11-23-audit/grade-after-fix.log) | 在唯一私有源码副本中执行原目标，仅更换GRADE_TMP保存位置；原注入器找不到init_main，运行停止，未开始评分 |

本地原始运行目录保存匹配的kernel ELF、ucore.img及文本证据。本分支仅上传构建日志、QEMU命令、console和摘要等文本，不上传生成的测试二进制，运行report/tests脚本可重建镜像。启动GDB证据见基础迁移目录的startup/gdb.log。总索引含源码、测试源和日志哈希，见[audit-index.json](evidence/2026-11-23-audit/audit-index.json)。

替身范围必须随报告保留：基础S/U寄存器探针链接实际trapentry，但使用受控测试trap；模块一U态探针则链接正式trap.c、syscall.c和实际用户封装。模块一的do_*是独立镜像的参数观察与受控返回探针，wait探针仅增加一次真实4字节复制检查，最后exit探针只选择帧返回位置；都没有实现真实进程业务。print_pgdir同样只是观察探针。用户探针直接装入测试二进制，使用专属合法Sv39页表，不是同伴的ELF装载，也不替生产get_pte证明硬件访问正确。

软件帧接口PASS和真实U态调用成功在以上证据中分开记录。原test_lab5_syscall只直接调用exception_handler，不含真实ecall；真实U态结果来自独立user镜像，不能相互冒充。原Process Create、Process Control及用户程序评分均未在本阶段通过。

## 六、后续接入顺序及回归清单

肖子昂可按模块二接口开始实现alloc_proc、kernel_thread、init_main、user_main、kernel_execve、do_execve、load_icode和do_wait。init_main缺失会阻止原注入器定位，这是待完成函数的直接依赖，不应删掉评分注入来绕过。用户装载动态运行还依赖杨昊冉的do_fork等函数，因此可以先实现与做隔离检查，不能据此要求A阶段已经提供完整用户进程。

杨昊冉接模块三copy_mm/do_fork/dup_mmap/copy_range及模块四do_exit/do_yield/do_kill。重点衔接父子关系、mm_count、页引用、子现场、错误回滚、等待唤醒和僵尸回收；不要重复定义或覆盖本人已经完成的sys_*。

集成验收前还需处理以下事项：

1. 明确print_pgdir缺项的补齐方式，由本人继续跟进模块一输出依赖；当前只有转发验证，不能计为真实页表打印完成。
2. 对齐生产get_pte非叶权限与原断言冲突。当前两者无法同时作为合法Sv39访问成立，保留证据，不能偷偷修改评分条件。这是开始真实用户映射前的阻塞。
3. 确认当前QEMU/OpenSBI与原grade设备加载方式的冲突；本轮grade尚未运行到QEMU，既有启动差异证据在基础迁移说明中，不能称本轮grade已验证其运行结果。
4. 所有后端就绪后，完整make、原注入检查及make grade必须重新运行；核对进程创建、ELF入口和用户栈、真实fork父子返回、wait退出码/回收、yield和kill。
5. 使用正式完整镜像重新演示用户ecall与sret，确认current->tf、sscratch和调度后内核栈仍一致；本阶段的独立镜像不是正式演示的替代品。
6. 检查资源页数、进程数和引用计数恢复，并在必做版本冻结后再做COW/Dirty COW等Challenge。完整双重GDB分支任务也尚未因本次普通内核GDB检查而完成。

本人报告可引用本文件中的原理、调用契约、审查表和原始日志，截图应从对应真实终端或GDB位置取得。本轮没有生成截图，也没有将日志渲染成仿真截图；最终截图与完整用户运行证据待后续单独整理。
