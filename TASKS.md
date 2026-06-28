# TASKS.md

本文件用于同步 MiniOB 向量检索系统的任务分工、完成状态和后续维护项。

## 任务分工

| 方向 | 负责内容 | 主要文件/目录 | 当前状态 |
| --- | --- | --- | --- |
| MiniOB 类型系统 | `VECTOR(N)` 类型、维度限制、向量值存储、比较规则 | `src/observer/common/`、`src/observer/storage/` | 已完成 |
| SQL 解析与绑定 | 向量类型语法、函数表达式、别名、排序、Limit、向量索引语法 | `src/observer/sql/parser/`、`src/observer/sql/stmt/`、`src/observer/sql/expr/` | 已完成 |
| 查询执行 | 函数计算、内存排序、Limit、向量索引扫描算子 | `src/observer/sql/operator/`、`src/observer/sql/optimizer/` | 已完成 |
| IVF_Flat 索引 | 索引元数据、确定性 K-Means、候选簇检索、插入/删除维护 | `src/observer/storage/index/`、`src/observer/storage/table/` | 已完成 |
| Flask 网关 | HTTP API、MiniOB plain 协议转发、表结构读取 | `backend/app.py`、`backend/requirements.txt` | 已完成 |
| React 前端 | SQL Terminal、Schema Panel、Vector Visualization、Benchmark View | `frontend/` | 已完成 |
| 回归测试 | 向量专项 SQL 用例和预期结果 | `test/case/test/vector-search.test`、`test/case/result/vector-search.result` | 已完成 |
| CI 与文档 | GitHub Actions、README、本地运行说明、协作文档 | `.github/workflows/`、`README.md`、`AGENTS.md`、`PROJECT_STATUS.md`、`TASKS.md` | 已完成 |

## 已完成任务

- 实现 `VECTOR(N)`、默认 `VECTOR` 维度 `2048`、最大维度 `16383`。
- 实现 `STRING_TO_VECTOR`、`VECTOR_TO_STRING`、`DISTANCE`。
- 支持 `EUCLIDEAN`、`COSINE`、`DOT`，兼容 `L2_DISTANCE`、`COSINE_DISTANCE`、`INNER_PRODUCT`。
- 支持 `SELECT ... AS alias`、`ORDER BY expr|alias [ASC|DESC]`、`LIMIT N`。
- 实现 `CREATE VECTOR INDEX` 和 IVF_Flat 默认/自定义参数。
- 优化器支持在匹配 `ORDER BY DISTANCE(...) LIMIT N` 时使用 `VECTOR_INDEX_SCAN`。
- 修复 IVF_Flat 按 `probes` 限定候选簇的检索边界问题。
- 增加 `vector-search` 专项回归并纳入 GitHub Actions。
- 完成 README 的课程项目说明、跨平台运行说明和关键代码位置说明。

## 后续维护项

| 优先级 | 任务 | 说明 |
| --- | --- | --- |
| P0 | 保持回归通过 | 后续代码改动后运行 `vector-search`，涉及公共逻辑时运行 `basic,vector-search` |
| P0 | 保持 README 与实际运行一致 | 端口、命令、目录或依赖变化后同步文档 |
| P1 | 补充实验报告材料 | 如需提交报告，可基于当前实现、测试输出和 README 整理 |
| P1 | 补充演示截图或流程说明 | 如老师需要查看运行效果，可记录建表、插入、Top-N 查询、索引执行计划 |
| P2 | 扩展性能对比数据 | 可增加不同数据规模、`lists/probes` 参数下的查询耗时对比 |

## 提交流程

1. 确认工作区状态：`git status -sb`。
2. 完成代码或文档修改。
3. 运行对应检查：构建、回归、前端构建或 `git diff --check`。
4. 提交相关文件。
5. 按需要推送到开发分支和 `main`。
