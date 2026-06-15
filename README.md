# MiniOB 向量检索后端

本仓库是《数据库系统设计实践》课程项目的后端实现，基于 OceanBase MiniOB `main` 分支扩展完成向量存储、向量距离计算、精确 Top-N 查询和 IVF_Flat 向量索引能力。

上游项目：<https://github.com/oceanbase/miniob>

## 实现范围

- 新增 SQL 类型 `VECTOR(N)`，支持默认维度 `VECTOR`、最大维度 `16383`、插入维度校验和向量等值比较。
- 新增函数 `STRING_TO_VECTOR`、`VECTOR_TO_STRING`、`DISTANCE`。
- 支持 `DISTANCE` 方法 `EUCLIDEAN`、`COSINE`、`DOT`，并兼容 `L2_DISTANCE`、`COSINE_DISTANCE`、`INNER_PRODUCT`。
- 扩展查询能力：`SELECT ... AS alias`、`ORDER BY expr|alias ASC|DESC`、`LIMIT N`。
- 新增内存排序与 Limit 物理算子。
- 新增 `CREATE VECTOR INDEX` 语法，支持默认参数和 `WITH (distance=..., type=ivfflat, lists=N, probes=M)`。
- 实现 IVF_Flat 向量索引元数据、确定性 k-means 训练、插入/删除维护和 `VECTOR_INDEX_SCAN` 执行路径。
- 优化器识别 `ORDER BY DISTANCE(vector_col, constant_vector, method) LIMIT N` 并在可用时下推到向量索引扫描，否则回退为全表扫描加排序。

## 示例 SQL

```sql
create table items(id int, emb vector(3), tag char);

insert into items values(1, string_to_vector('[1, 0, 0]'), 'a');
insert into items values(2, string_to_vector('[0, 1, 0]'), 'b');
insert into items values(3, string_to_vector('[0, 0, 2]'), 'c');

select id, vector_to_string(emb) as emb_text
from items
where emb = string_to_vector('[1,0,0]');

select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis
from items
order by dis asc
limit 2;

create vector index idx_items_emb on items(emb)
with (distance=euclidean, type=ivfflat, lists=245, probes=5);

explain select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis
from items
order by dis asc
limit 2;
```

## 关键代码位置

- 类型和值系统：`src/observer/common/value.*`、`src/observer/common/type/vector_type.*`
- SQL 语法解析：`src/observer/sql/parser/lex_sql.l`、`src/observer/sql/parser/yacc_sql.y`
- 表达式与函数：`src/observer/sql/expr/expression.*`
- 查询绑定与计划：`src/observer/sql/parser/expression_binder.*`、`src/observer/sql/optimizer/*`
- 排序/Limit 算子：`src/observer/sql/operator/sort_*`、`src/observer/sql/operator/limit_*`
- 向量索引：`src/observer/storage/index/ivfflat_index.*`
- 向量索引扫描：`src/observer/sql/operator/vector_index_*`
- 回归脚本：`test/case/test/vector-search.test`

## 构建与测试

推荐按课程资料包中的 WSL2 + Docker / MiniOB 官方开发环境构建：

```bash
bash build.sh debug --make -j"$(nproc)"
```

向量功能回归脚本：

```text
test/case/test/vector-search.test
```

当前本地 Windows 环境缺少可直接使用的 WSL 发行版、Docker、CMake、Flex/Bison，因此本仓库提交前完成了 Git diff 格式检查，完整编译和集成测试应在课程指定 Linux/Docker 环境中执行。

## 许可证

本项目继承 MiniOB 的 Mulan PSL v2 许可证。详见 `License`。
