# uCore Lab5：用户程序

本分支为小组实验五的阶段交付。

- `code/`：目标Lab5源码，已完成必要迁移、双向Trap及模块一分发和适配。
- `report/report.md`：按课程模板填写的李华濂阶段报告；其他成员章节待填写。
- `report/prompt.md`：实际记录暂停状态说明，未补造对话。
- `report/images/`：实际截图待补清单。
- `report/tests/`、`report/evidence/`：独立检查入口及原始文本证据。
- [阶段交接](report/成员A阶段交接说明.md)：接口、版本和后续任务。

肖子昂接模块二，杨昊冉接模块三、四。完整内核和grade当前尚未通过；缺失进程函数、print_pgdir及框架页表冲突见报告。独立镜像结果不等于真实进程业务通过。

```bash
conda activate os2026
python3 report/tests/foundation/run.py
python3 report/tests/module_one/run.py
python3 report/tests/module_one/run.py --timer-only
```

生产构建使用`cd code && make`，当前因未完成模块而链接失败。独立检查只在临时镜像使用明确标注的后端探针，正式源码中没有空后端。
