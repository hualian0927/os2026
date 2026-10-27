# 成员A报告补充验证与截图

日期：2026-10-27。正式源码基于提交`8878a8c`；本轮只新增报告、工具、截图和证据，`code/`的内容哈希未变。未新增实际提示词记录。

## 本轮真实执行

在分支根目录执行了`python3 report/tools/collect_member_a.py`，结果如下：

- 实际启动、DTB解析、PMM、Best-Fit、保留页与SIE0/1检查通过。
- 模块一O0/O2各64次真实Trap通过，两个故意错误汇编副本均被明确检出。
- GDB实际检查启动参数、栈、两个根映射、管理器及内存状态。
- GDB实际检查第3次Trap的帧布局、原sp、修改后的epc和a0/s0/s1/t0带回，以及返回状态位。

入口会拒绝覆盖已有`collection.json`，以免把历史证据静默替换。再次验证模块一可直接使用原入口：

```bash
conda activate os2026
cd /path/to/os2026-lab3
python3 report/tests/module1/run.py --source "$PWD/code" --negative-controls
```

## 文件索引

| 文件 | 内容 |
|------|------|
| [collection.json](collection.json) | 源码提交、全部正式文件哈希、临时构建目录、原结果位置和检查范围 |
| [foundation-build.log](foundation-build.log) | 基础独立镜像的真实编译、链接和objcopy命令 |
| [foundation-run.log](foundation-run.log) | 真实基础镜像的完整串口输出 |
| [module1-run.log](module1-run.log) | O0/O2及负面对照脚本输出 |
| [module1](module1/) | 本轮模块一完整构建日志、各镜像串口、错误副本和结构化结果；不含可执行文件 |
| [boot-gdb.log](boot-gdb.log)、[boot-gdb.gdb](boot-gdb.gdb) | 实际启动/内存调试输出及当时命令 |
| [frame-gdb.log](frame-gdb.log)、[frame-gdb.gdb](frame-gdb.gdb) | 实际帧/返回调试输出及当时命令 |
| [screenshot-manifest.json](screenshot-manifest.json) | 五张截图的源日志、片段边界、原始日志哈希、图片哈希与尺寸 |

随后执行`python3 report/tools/capture_member_a.py`，在已有X11桌面上打开真实只读日志窗口并抓取窗口内容，生成`report/images/A01`至`A05`。没有绘制终端或编辑图片；截取片段的界限在清单中，原始输出保持不变。原始SBI串口含少量控制字节，GitHub可能按二进制显示，可下载后使用`rg -a`查看；脚本汇总日志可直接阅读。

本轮生成的ELF、镜像和对象文件不纳入Git。完成截图后，模块一运行目录归档到本次`/tmp`构建目录，原位置及归档位置均记录在`collection.json`；历史GDB脚本保留当时路径，换机器重现时应重新构建并加载对应ELF。

## 边界

基础镜像使用专用kern_init、panic和意外Trap停止点；模块一使用专用kern_init及最小trap(tf)。均链接正式被测模块，不覆盖正式trap.c，不实现通用handler。动态SIE检查清零sie来源；未运行正式完整启动、make grade、连续时钟或计数关机。13个初始化边界用例与512步Best-Fit回归只引用此前原始结果，没有声称本轮重跑。截图不是IDE终端画面，报告已注明其日志查看窗口来源。
