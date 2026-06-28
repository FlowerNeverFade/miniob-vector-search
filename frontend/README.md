# MiniOB 向量检索前端控制台

这是课程演示项目的 React Vite 前端，配合根目录下的 Flask 网关后端和 MiniOB Observer 使用。

## 功能

- SQL Terminal：执行 MiniOB SQL，并展示执行结果。
- Schema Panel：查看表、字段和向量索引信息。
- Vector Visualization：对向量数据做二维可视化展示。
- Benchmark View：对比精确扫描与索引检索的 Top-K 结果。

## 本地运行

先确认 MiniOB Observer 和 Flask 网关已经启动：

- MiniOB plain 协议端口：`localhost:6789`
- Flask 网关：`http://localhost:5000`

然后启动前端：

```bash
npm install
npm run dev
```

浏览器打开：

```text
http://localhost:5173/
```

## 构建

```bash
npm run build
npm run preview
```
