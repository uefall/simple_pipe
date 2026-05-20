# simple_pipe

**CSPF（Configurable Stream Processing Framework）** 的 C++17 实现：声明式 DAG、异步算子节点、`FrameMeta` 帧载体、Build / Runtime 双平面配置。架构抽象自 PM-Pipeline。

## 构建（CMake + GTest）

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## 运行示例

```bash
./build/samples/run_demo_cpp
./build/samples/run_five_node_100_cpp
```

## 目录结构

```text
include/simple_pipe/     # 公共头文件
src/                     # 库实现
configs/demo/graph/      # 拓扑 JSON（linear / five_node / safety_fanin）
configs/demo/node_defaults/
tests/                   # GTest：builder、pipeline、多 graph
samples/                 # C++ 可执行样例
```

## 对外 API（会话式）

```cpp
simple_pipe::SimplePipeline pipeline("configs/demo/node_defaults");
pipeline.BuildFromJson("configs/demo/graph/five_node.json");
pipeline.Configure(runtime);
pipeline.SetOutputCallback([](const FrameMeta& m) { /* ... */ });
pipeline.StartPipeline();
pipeline.PushFrame(input);
pipeline.StopPipeline();
```

## 示例 Graph

| 文件 | 拓扑 |
|------|------|
| `linear.json` | app_src → resize → app_des |
| `five_node.json` | app_src → resize → detector → rules → app_des |
| `safety_fanin.json` | 检测 ∥ 运动 → tracker → rules → sink |

## 扩展算子

1. 继承 `Operator`，实现 `role()` 与 `HandleFrameMeta()`
2. 在 `src/operators/node_registry.cpp` 注册 `type`
3. 在 `configs/.../graph/*.json` 引用

## 文档

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

## 说明

仓库中保留的 Python 实现（`simple_pipe/` 包、`pyproject.toml`）为早期参考，**主构建与测试以 C++ 为准**。
