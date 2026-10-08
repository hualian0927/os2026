# Lab 1 working instructions

- 当前交付位于 `lab1` 分支，根目录只有 `code/` 和 `report/`；本文件所在目录为 `code/`。
- 作者：2412351－李华濂。提交标题以 `李华濂：` 开头，不添加 AI 署名。
- 本次沿用用户已安装的 GDB；Makefile 保持资源包原样，不擅自恢复兼容性修改。
- 源码核验使用 `make`；本地兼容性自检使用 `python3 tools/check_lab1.py --gdb gdb-multiarch`，内部采用 `-kernel`。
- 原版 `make grade` 缺少 `check` 目标，原版 loader 参数在当前固件下未到达内核；报告必须保留这些限制。
- 测试结果不能冒充官方评分；不得把旧版、原始代码与当前代码的证据混用。
- 报告在 `report/report.md`，实际提示词在 `report/prompt.md`，截图在 `report/images/`。
- 不上传口令、令牌、构建产物；不编造截图或运行结果。
- 用户要求截图补齐后再统一推送；本轮先本地提交，不推送。
- 保留 main 分支及其工作目录中的本地改动，不强制推送或改写既有历史。
