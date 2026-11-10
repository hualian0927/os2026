# Lab4模块一独立检查

本检查链接真实`pmm.c`、Best-Fit、`entry.S/trapentry.S`及未修改的`tools/test_lab4.c`。测试支架只提供独立C启动入口、panic及意外Trap退出，不定义任何生产进程或调度函数，不把替身写进正式代码。

```bash
conda activate os2026
cd /home/lhl/os2026

# 原框架软件检查、状态变化及内存不足回滚：O0/O2
python3 lab4/tests/page_mapping/run.py --mode software

# 对调用者事先提供的规范页表进行真实别名及映射替换：O0/O2
python3 lab4/tests/page_mapping/run.py --mode prebuilt

# 所有检查；当前会报告两个硬件失败，整体退出码1
python3 lab4/tests/page_mapping/run.py
```

`--mode generated`只检查生产get_pte新建路径的真实访存；`--mode framework`只运行未修改的check_boot_pgdir，二者当前均失败。每次运行在`report/evidence/2026-11-10-page-mapping/checks-*`创建新目录，并记录匹配镜像、ELF、构建命令、源文件摘要和日志。不会用预期失败标记把这两个失败转成PASS。

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

范围限制：未验证权限收紧后的实际写故障、删除后访存故障、全部跨索引组合、用户模式、多hart或完整调度链。原check_boot_pgdir的int循环问题未改，不能宣称内核映射遍历已覆盖。测试使用单hart、关闭异步中断的串行场景。
