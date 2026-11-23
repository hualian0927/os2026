# Lab5测试截图待补

当前提交没有实际截图，不用日志图片代替终端截图。以下位置按真实结果取图后，再在report.md中插入相对图片引用。

| 建议文件名 | 截取内容 | 限定范围 |
|---|---|---|
| foundation-checks.png | python3 report/tests/foundation/run.py的结果 | 独立基础镜像，不是正式启动 |
| syscall-interface.png | 模块一interface-O2/console.log对应实际终端的断言结果 | 软件构造Trap帧，不是U态ecall |
| user-syscall.png | 模块一真实user-O2检查与USER_SYSCALL_PASS | 实际U态链，do_*为观察探针 |
| timer-before-after.png | 已保留的修复前超时与修复后USER_TIMER_PASS真实运行界面 | 时钟通知修复，不是真实父子切换 |
| formal-qemu.png | 集成后的make qemu | 待后续模块完成后取得 |
| formal-grade.png | 原make grade的完整结果 | 待完整评分后取得，不把注入失败当通过 |

原始console和构建日志在report/evidence中；本轮未取得截图，以上文件尚不存在。
