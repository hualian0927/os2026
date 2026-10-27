# 成员A阶段证据索引

这些文件来自2026-10-27已经执行的检查。本次交接只归档原始材料，没有重跑测试，没有替换日志中的历史路径。阶段总说明在[成员A阶段交接说明](../成员A阶段交接说明.md)。

| 材料 | 内容与范围 |
|---|---|
| [stage-manifest.json](stage-manifest.json) | A1源码完整SHA-256、六个改动文件、Lab2迁移来源版本、关键内容未改的比较结果 |
| [source-diff.patch](source-diff.patch) | 当前正式代码相对lab3.zip初始代码的完整文本差异 |
| [log-provenance.json](log-provenance.json) | 本目录归档的30个证据文件原路径及哈希 |
| [before-loader.log](foundation/before-loader.log)、[before-kernel.log](foundation/before-kernel.log) | 同一隔离镜像在两种加载方式下的固件入口对照 |
| [before-page-fault.log](foundation/before-page-fault.log) | 未增加恒等映射时的真实取指页错误，cause=0xc、epc/tval=0x80200038 |
| [after-paging-gdb.log](foundation/after-paging-gdb.log) | 实际入口寄存器、DTB魔数、根表项、内核栈与内存初始化调试记录 |
| [after-foundation.log](foundation/after-foundation.log) | 真实启动、DTB、PMM、Best-Fit和SIE0/1检查；专用kern_init，不是完整Lab3 |
| [make-debug.log](foundation/make-debug.log)、[make-gdb.log](foundation/make-gdb.log) | 匹配隔离ELF/镜像上的Makefile调试入口检查 |
| [page-init.log](foundation/page-init.log) | 13个独立边界用例；DTB getter和分配器回调由测试提供 |
| [best-fit.log](foundation/best-fit.log) | O0/O2管理器检查汇总，各512步位图核对 |
| [best-fit-console-O0.log](foundation/best-fit-console-O0.log)、[best-fit-console-O2.log](foundation/best-fit-console-O2.log) | Best-Fit详细结果，包含三个真实公共包装的SIE保存/恢复探针结果 |
| [grade-loader-console.log](foundation/grade-loader-console.log)、[grade-kernel-console.log](foundation/grade-kernel-console.log) | 评分加载参数的隔离对照，不是评分通过证明 |
| [module1-first-run](module1-first-run/) | 模块一首次O0/O2各8次Trap检查、汇编编译和正式构建阻塞日志 |

最新O0/O2各64次Trap、两项负面对照及原始输出，位于[独立模块一测试材料](../tests/module1/README.md)。正式完整构建的最后已有记录是[module1-first-run/full-build.log](module1-first-run/full-build.log)，阻塞原因是两个模块二handler缺失。

`foundation/validation.json`属于前置适配阶段，当时模块一尚未实现；`module1-first-run/validation.json`属于随后完成模块一的阶段。应按时间和范围使用，不能把早期未覆盖声明当作最新结果，或把独立通过写成正式评分通过。

打包不携带对象文件、可执行ELF、QEMU镜像、Windows下载元数据或旧提示词文档。需要重建时，使用阶段说明给出的测试入口与显式源码路径。
