# 2026-11-20 报告与源码核对

本目录记录本次报告整理实际进行的检查，区别于2026-11-10的历史运行证据。

- `report-before-review.md`：修改前报告的原文快照；其中相对路径仍以原report目录为基准。
- `review.json`：当前源码摘要、工具可用性、目标文件编译尝试、四个核心C文件前端语法检查、历史源码对比和文档链接检查。

当前PATH缺少RISC-V GCC/LD、QEMU和gdb-multiarch。Apple Clang无法生成RISC-V目标文件，另外严格诊断指出原kmalloc.c的整数到指针参数转换。因此没有得到正式目标文件、链接、启动或评分通过结论，也没有将这些失败归因为线程代码缺失。

proc.c、sched.c、pmm.c、vmm.c采用 `clang --target=riscv64-unknown-elf -fsyntax-only` 的前端检查均返回0。proc.c和pmm.c的原static_assert宏各产生一处布尔switch警告；前端通过不能替代目标汇编、链接或运行。

当前源码相对既有packaging-check记录只在proc.c、sched.c、switch.S发生变化。本次报告工作未改变生产源文件，未修改测试断言、历史日志或十张历史截图。README、当前报告及答辩指南的相对链接均指向存在的文件。

后续集成记录：已修复kmalloc参数转换、Sv39非叶子标志及对应父项断言、评分线程回调签名。用户提供的Debian修后启动输出已到init，评分PMM/Process Create/Schedule均OK，30/30。相应user-provided.log与provenance文件保存在本目录。上述“未改源码”和“没有运行结果”仅描述初次核对阶段，不是最终状态；Mac未执行Debian运行测试。
