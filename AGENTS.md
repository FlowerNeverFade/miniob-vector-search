# AGENTS.md

本文件用于统一 MiniOB 向量检索系统的开发、测试、文档和同步规则。适用范围为仓库全部目录。

## 项目定位

- 本仓库是《数据库系统设计实践》课程项目 MiniOB 向量检索系统。
- 项目包含 MiniOB 内核扩展、Flask 网关后端和 React Vite 前端界面。
- README 与仓库描述使用课程项目口径，避免宣传式、包装式或明显自动生成式表述。
- 未经明确要求，不修改前端可见名称、浏览器标题和 `frontend/package.json` 项目名。

## 开发规则

- 保持变更聚焦，优先沿用 MiniOB 现有代码风格和目录组织。
- 不回退或覆盖他人已有改动；遇到非本次任务相关的工作区变更时保留原状。
- C++ 代码注释只写必要说明，重点解释复杂逻辑、边界条件和课程要求对应点。
- 文档使用 UTF-8 与 LF 换行，内容保持正式、清楚、可复现。
- Windows 下执行 npm 命令优先使用 `npm.cmd`，避免 PowerShell 执行策略拦截 `npm.ps1`。
- MiniOB Observer 必须以 `plain` 协议运行在 `6789` 端口，Flask 网关默认监听 `5000`，前端默认监听 `5173`。

## 测试规则

内核变更至少运行：

```bash
bash build.sh debug --make -j"$(nproc)"
python3 test/case/miniob_test.py --test-cases=vector-search
```

涉及基础 SQL、优化器、存储或索引公共逻辑时继续运行：

```bash
python3 test/case/miniob_test.py --test-cases=basic,vector-search
```

前端变更至少运行：

```powershell
npm.cmd --prefix frontend run build
```

Linux/macOS 可使用：

```bash
npm --prefix frontend run build
```

文档变更至少运行：

```bash
git diff --check
```

## Git 与同步规则

- 每次提交只包含同一目的的相关文件。
- 提交信息使用简短英文动词短语，例如 `Document cross-platform runtime`。
- 需要同步 GitHub 时，推送当前工作分支，并按任务要求同步到 `main`。
- 推送前确认 `git status -sb`，避免把临时文件、构建产物或无关改动提交。

## 课程功能边界

核心能力围绕四个任务维护：

- A1：`VECTOR(N)` 类型、默认维度、最大维度、向量存储、插入校验和比较规则。
- A2：`STRING_TO_VECTOR`、`VECTOR_TO_STRING`、`DISTANCE` 与距离错误处理。
- A3：`SELECT ... AS`、`ORDER BY expr|alias [ASC|DESC]`、`LIMIT N`。
- A4：`CREATE VECTOR INDEX`、IVF_Flat、`lists/probes`、Top-N 检索和 `VECTOR_INDEX_SCAN`。

新增能力必须配套 SQL 回归用例，优先放入 `test/case/test/vector-search.test` 与 `test/case/result/vector-search.result`。
