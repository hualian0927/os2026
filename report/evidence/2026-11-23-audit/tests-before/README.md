# Lab5模块一独立检查

这些检查链接实际模块一代码，不是完整Lab5、正式进程运行或官方评分结果。正式源码没有加入任何测试后端。

在仓库根目录执行：

```bash
conda activate os2026
python3 lab5/tests/module_one/run.py
```

脚本复用`os2026`工具路径，分别检查`-O0`和`-O2`，每个QEMU运行有8秒超时，仅终止自己创建的进程。日志、命令、独立ELF和镜像保存在`lab5/evidence/2026-11-23-module-one/run-*/`，最近一次目录见该证据目录的`latest.txt`。

## 被测实现与观测

| 镜像 | 实际被测内容 | 观察方式 |
|---|---|---|
| interface | 实际syscall.c、trap.c、trapentry.S、vmm.c中的user_mem_check/find_vma；原tools/test_lab5.c中的test_lab5_syscall不改动 | 原断言；非法编号不触发后端；参数逐项比对；有符号返回；原current->tf恢复；后端修改EPC/SP/s2/status保留 |
| user | 实际user/libs/syscall.c、正式trapentry/trap/exception_handler/syscall、真实页分配器与用户检查/复制函数 | 在真实U态连续16轮调用8个现有公开用户封装，检查正负返回、内核实际收到的参数、用户栈哨兵、真实4字节copy_to_user，以及返回U态继续执行和最后按修改后的帧返回S态 |

interface还检查32项分发表的空洞和完整64位编号、五个64位参数槽、整数范围；NULL等待地址、4字节边界、只读VMA、跨空洞、栈保护页、内核地址、mm为空和范围溢出的拒绝。五参数检查只在该测试镜像内暂时替换一个静态分发表项为观察函数，之后恢复；不修改生产文件。

## 支架和覆盖边界

- interface使用软件构造的Trap帧和测试VMA，调用真正的trap/exception_handler；其低地址缓冲区未建立硬件映射，只检查验证与转发，不执行这些指针的复制。
- user使用实际内存初始化与分配器、真实DTB，并用测试专属的合法Sv39页表装入独立用户探针二进制。这不是同伴的ELF装载器，也不验证生产get_pte的非叶权限问题。
- user的入口脚手架复用`tests/foundation/user.S`，该文件未修改。用户探针和实际用户库独立编译链接，编译使用`rv64gc`、禁用小数据和链接松弛，以便使用明确的测试入口和无需重定位的原始镜像。生产构建参数未修改。
- 所有do_*都是仅在独立镜像内存在的调用探针：观察输入并返回指定测试结果；wait探针只测试真实copy_to_user，未实现等待、父子关系或资源回收。最后一次exit探针主动修改本次有效帧，检验正式返回链采用修改后的PC、SP和特权级，未实现退出业务。
- print_pgdir只有测试调用观察函数，不验证真实打印。正式框架目前声明该函数但缺少定义，完整内核仍因此及同伴未完成函数而链接失败。
- 原评分测试仅运行其中Syscall检查；Process Create、Process Control和完整grade没有据此通过。真实fork/exec/wait/exit/kill、调度、恶意访存异常、COW和SMP不在本次通过范围。
- user没有用户sys_exec封装，exec通过interface测试验证参数、失败码及帧修改；不能称为真实用户exec成功。

脚本在运行前后比较生产文件哈希；已有评分测试、Trap帧布局、SAVE_ALL和生产汇编均保持不变。原始用户封装及其反汇编对比另存于同级证据目录，未用修正后的测试结果反推旧版本一定崩溃。
