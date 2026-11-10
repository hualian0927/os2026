# Lab4模块一独立检查

本检查链接真实`pmm.c`、Best-Fit、`entry.S/trapentry.S`及未修改的`tools/test_lab4.c`。测试支架只提供独立C启动入口、panic及意外Trap退出，不定义任何生产进程或调度函数，不把替身写进正式代码。

```bash
conda activate os2026
cd /home/lhl/os2026

# 原框架软件检查、状态变化及内存不足回滚：O0/O2
python3 lab4/tests/page_mapping/run.py --mode software

# 对调用者事先提供的规范页表进行真实别名及映射替换：O0/O2
python3 lab4/tests/page_mapping/run.py --mode prebuilt

# 本轮扩展集：软件矩阵/失败回滚/跨索引，以及两个隔离访存故障用例
python3 lab4/tests/page_mapping/run.py --mode extended \
    --evidence-dir lab4/report/evidence/2026-11-10-independent-tests

# 所有检查；当前会报告两个硬件失败，整体退出码1
python3 lab4/tests/page_mapping/run.py
```

`--mode generated`只检查生产get_pte新建路径的真实访存；`--mode framework`只运行未修改的check_boot_pgdir，二者当前均失败。`--mode protect`和`--mode unmap`分别检查预期的存储、加载页异常。每次运行在证据目录中创建新的`checks-*`目录，记录匹配镜像、ELF、构建命令、源文件摘要和日志。已增加测试支架快照与摘要记录。不会用预期失败标记把原先两个非预期硬件失败转成PASS。

软件检查包括：原check_pgdir、原评分PMM测试、非页对齐地址、原样保留0/W等课程标志、同页重映射、共享后替换、删除幂等、数据页与测试自行回收页表后的页数恢复。分配失败通过真实耗尽物理页制造，不替换分配器：

1. 无空闲页，新建上级页表失败。
2. 仅余一页，先新建上级页表，下一次分配失败；验证父项完整旧值恢复、页数归还、输入页ref不变。
3. 已有上级页表及旧映射，无空闲页创建另一个末级表失败；验证旧父表、旧PTE和引用不被释放或更改。

硬件检查分为两个独立镜像：

- prebuilt：由测试作为调用者预先分配两张普通页表，初始非叶子项使用现有ptd_create产生的V位；再调用真实page_insert/page_remove。不是临时清除生产get_pte生成项的U位，也不是完整生产启动证据。
- generated：让真实get_pte从空分支创建两张表，使用与prebuilt相同的RWAD叶子权限。当前QEMU读取0x4000时产生加载页异常；非叶子项低位为0x11，仍有U。设置A/D为1排除了本次探针因叶子A/D为0引起故障的混淆。

实际综合运行`checks-birpehib`的结果：software-O0/O2和prebuilt-O0/O2通过；generated-O2和framework-O2失败。前者读取0x4000时cause=13，后者在原check_boot_pgdir写入0x100时cause=15。完整摘要中all_passed=false，脚本退出码1。

原check_pgdir要求非叶子项带U，而严格Sv39要求清除该保留位；本轮没有改断言或生产非叶子权限。这一框架兼容性问题仍未解决，不能把prebuilt结果写成“生产自动建表已通过”。

首次探针失败证据在`run-4wdontj_/O0`，当时的测试源码单独保存在`first-attempt-check.c`；后续将测试分层并保留原失败，而非覆盖它。

## 本轮扩展结果

`report/evidence/2026-11-10-independent-tests/checks-igrjnwj6`保存了6个通过镜像：software、protect、unmap各-O0/-O2。软件检查新增已映射输入页在OOM后保留引用及PTE、同页标志替换矩阵、跨2 MiB/1 GiB索引及全部页数恢复。

预期异常由新增`access_probes.S`中的真实sd/ld指令触发。测试Trap观察器必须匹配cause、stval、准确故障PC及S态状态，再仅修改测试帧epc返回指定后继；返回后还检查实际数据、引用及空闲页数。没有发生异常或发生在错误位置都失败。Trap汇编仍为正式trapentry.S，不替换生产handler文件。

- protect：先成功写入，降为只读后确认读仍成功，再写应发生cause=15；物理页内容、ref及页数应保持。追加“只读仍可读”的正向对照后，仅重跑权限镜像-O0/-O2，记录在`checks-3txefomy`，均通过。
- unmap：先成功读取，删除映射并使同一物理页被重新分配写入不同值，再读旧VA应发生cause=13，不能读到复用页。

两个异常用例分别启动独立QEMU，单个镜像10秒超时，实际cause与地址均符合预期。所有本轮代码在SIE=0、sie=0下测试，未开启未受控的异步来源。6镜像初次运行的支架版本也已在其目录下保留，未覆盖旧失败证据。

范围限制：这些硬件用例使用调用者预置规范路径，仍不证明生产get_pte自动创建路径通过。未覆盖全部跨索引组合、用户模式、多hart或完整调度链。原check_boot_pgdir的int循环问题未改，不能宣称内核映射遍历已覆盖。此前的初始化、保留区、SIE=0/1及隔离panic结果按相关实现未变复用原日志，不计作本轮新运行。
