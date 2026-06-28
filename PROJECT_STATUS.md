# PROJECT_STATUS.md

最后更新：2026-06-28

## 当前状态

MiniOB 向量检索系统已完成课程四个后端任务，并配套 Flask 网关后端、React Vite 前端界面、专项 SQL 回归用例和 GitHub Actions 验证流程。

当前仓库：

- GitHub：`FlowerNeverFade/miniob-vector-search-backend`
- 主要分支：`main`
- 开发分支：`codex/vector-search-backend`
- MiniOB 上游基线：`oceanbase/miniob` main 分支 commit `9f856a542decb6dc678650406af7d6e351940dab`

## 功能进度

| 模块 | 状态 | 说明 |
| --- | --- | --- |
| A1 向量类型数据存储 | 已完成 | 支持 `VECTOR(N)`、默认 `VECTOR` 维度、最大维度限制、插入维度校验和向量等值比较 |
| A2 向量距离计算 | 已完成 | 支持 `STRING_TO_VECTOR`、`VECTOR_TO_STRING`、`DISTANCE`，覆盖欧氏距离、余弦距离和内积 |
| A3 精确查询与排序 | 已完成 | 支持 `SELECT ... AS`、`ORDER BY` 字段/函数/别名、升降序和 `LIMIT` |
| A4 IVF_Flat 近似搜索 | 已完成 | 支持 `CREATE VECTOR INDEX`、默认/自定义参数、确定性 K-Means、Top-N 下压和 `VECTOR_INDEX_SCAN` |
| Flask 网关后端 | 已完成 | `backend/app.py` 负责 HTTP 到 MiniOB plain 协议的转发 |
| React Vite 前端 | 已完成 | `frontend/` 提供 SQL 控制台、表结构、向量可视化和检索结果展示 |
| README 文档 | 已完成 | 已写入课程项目说明、SQL 功能、关键代码位置、Windows/Linux/macOS 运行步骤 |

## 已验证内容

本地已验证：

- MiniOB Debug 构建通过：`bash build.sh debug --make -j4`
- 专项回归通过：`python3 test/case/miniob_test.py --test-cases=vector-search`
- 基础与专项回归通过：`python3 test/case/miniob_test.py --test-cases=basic,vector-search`
- Flask 网关可连接本地 MiniOB Observer
- React Vite 前端可在 `http://localhost:5173/` 运行

远端已配置 GitHub Actions：

- Ubuntu Debug build + CTest
- Ubuntu Release build
- macOS build
- `basic-test`，包含 `basic` 与 `vector-search`
- `integration-test`
- `memtracer-test`
- `benchmark-test`
- sysbench 矩阵

## 本地运行约定

- Windows：推荐 WSL2 Ubuntu-24.04 构建并运行 MiniOB Observer，Windows 主机运行 Flask 与 React 前端。
- Linux：可直接原生构建 MiniOB、启动 Flask 和前端。
- macOS：可直接本机构建运行；若需贴近课程环境，优先使用 Linux 虚拟机或容器。
- 端口约定：MiniOB `6789`，Flask `5000`，Vite `5173`。

## 当前注意事项

- 本项目本地完整运行依赖真实 MiniOB Observer，不使用 mock 演示模式。
- 前端可见标题和 `frontend/package.json` 名称保持原样，除非后续明确要求修改。
- 文档内容保持课程项目口径，避免使用不适合老师直接查看的包装式描述。
- 若后续补写实验报告或演示材料，应以 README、测试结果和专项 SQL 用例为依据。
