# 最新版本运行复核与截图覆盖

日期：2026-10-22。拉取基线：`2c5519aa38bc0cc9f52f2a6e5a6c5d6d20d8c767`。

## 执行范围

本轮在 `lab2` 分支工作目录 `/home/lhl/os2026-lab2` 执行 `git pull --ff-only origin lab2`，以远端版本为基础补充报告和证据。生产代码、题目测试、原有11张截图及 `report/prompt.md` 均保持拉取后的内容，本轮不计入实际提示词。

新截图12～23由本次真实命令/GDB输出生成：将原始日志的指定段落显示到可见的 Qt 日志窗口，再用 QWidget.grab 抓取窗口。它们是日志窗口截图，不冒充成员手动操作的终端截图，不拼写或修改 PASS、变量值、断言和返回值。完整日志与截取位置一并保存，见 `screenshot-manifest.json`。原图1～11保留历史和成员来源。

## 运行结果

| 检查 | 结果 | 日志 |
|------|------|------|
| 默认编译 | 成功 | build-best-fit.log、build-final-best-fit.log |
| 默认内核真实启动 | DTB、Best-Fit及自测输出齐全 | boot-best-fit.log |
| 题目评分 | page init、best fit均OK，20/20 | make-grade.log |
| page_init | 13个隔离用例通过 | check_page_init.log、check-page-init/ |
| Best-Fit | O0/O2及各512步模型核对通过 | check_best_fit.log、check-best-fit/ |
| Best-Fit反向检查 | 正常实现通过，两种临时错误被断言拒绝 | check_best_fit_review.log、check-best-fit-review/ |
| Buddy隔离检查 | O0/O2及影子模型检查通过 | check_buddy.log、check-buddy/ |
| Buddy真实启动 | 选择Buddy、内置自测和初始化成功 | boot-buddy.log |
| 客户机GDB | 分页前后、根叶项和别名一致 | guest-mmu.log、guest-mmu.gdb |
| 宿主QEMU 4.1.1 GDB | Bare、PTE单步、victim未命中、回填和主表命中 | host-mmu.log |

普通启动和评分使用QEMU 8.2.2/OpenSBI 1.3；宿主源码调试使用本机QEMU 4.1.1调试构建及其自带的OpenSBI 0.4，客户机复核另用QEMU 8.2.2。不同版本日志分开保存，未将一台机器的局部变量当作另一台机器的观测结果。

Buddy与默认构建切换时均先清理，避免复用不同编译宏生成的对象；结束时已恢复默认Best-Fit构建。自动采集的QEMU在初始化标记齐全后由脚本主动结束，所以日志末尾的signal 15是收尾，不是启动失败。隔离检查的替身范围沿用各脚本原有说明。

## 页表与TLB的直接证据

- Bare：mmu_idx=1，satp=0，输入0x80200000，返回相同物理页基址；调用栈仍经过软件TLB慢路径。
- Sv39：输入0xffffffffc02000d6，根物理地址0x80205000，idx=511，PTE地址0x80205ff8，值0x200000cf，levels=3但第一轮就是根级叶项。
- 单步：next经过V位及R/W/X检查，到叶项地址计算后finish得到返回码0、权限7。QEMU 4.1.1该接口返回4KiB物理页基址0x80200000，包含低12位偏移的字节物理地址为0x802000d6。
- victim：实际FinishBreakpoint记录第一次高地址查询返回false。函数入口断点本身不能证明命中；报告已修正旧表述。
- 回填：在tlb_set_page调用前直接读取输入地址、pa、ret、prot和mmu_idx。
- 主表命中：entry->addr_code等于addr按4KiB对齐的值；比较结果为1，next直接跳过victim/fill分支。这是直接分支观测，不是仅以没有再次命中fill断点推测。

## 截图覆盖

| 图 | 覆盖 |
|----|------|
| 12 | 最新默认内核完整启动，补充历史图7没有的内核输出 |
| 13 | 最新题目评分20/20 |
| 14、15、21 | Buddy隔离检查、真实启动和完整回收 |
| 16 | 客户机分页前后、PTE、反汇编别名 |
| 17 | Bare路径及调用栈 |
| 18 | PTE局部变量、分支单步与叶项结果 |
| 19、20、23 | victim返回false、TLB回填和主表比较命中 |
| 22 | 最新代码的初始化及Best-Fit回归 |

所有必做练习已有相应证据。内存范围获取Challenge是思考题；成员B提供的SLUB文件只有两份提示词，本次拉取时没有对应实现分支，因此不编造SLUB运行结果或命令。该可选实现仍需单独交付才能进行完整实现/运行验收。

本目录保留各项原始运行输出和截图摘录清单。`validation.json` 记录链接、来源哈希、源代码不变等检查结果。
