# CSPF 架构说明（simple_pipe）

本仓库将 **PM-Pipeline** 文档中的原型落实为 **C++17** 库（CMake + GTest），命名仍为 **CSPF — Configurable Stream Processing Framework**。

## 核心范式

- **Pipeline-and-Filter**：节点链式处理 `FrameMeta`
- **DAG**：`PipelineGraphSpec.edges` 描述 fan-in / fan-out
- **Observer**：`meta_flow` 推送至下游 `in_queue`
- **Facade**：`SimplePipeline` 对外隐藏 builder 与连线

## 会话状态机

```text
Unbuilt → Build → Built → Configure → Configured → Start → Running
Running → PushFrame* → Running → Stop → Stopped
```

## 数据平面

```text
PushFrame → AppSource.inject → out_queue → dispatch → downstream.meta_flow
         → in_queue → handle_frame_meta → out_queue → … → AppSink → callback
```

Fan-in 节点（如 `mock_tracker`）按 `frame_index` 收集各上游 `_upstream_id` 的 meta，凑齐后合并再处理。

## 配置双平面

| 平面 | API / 文件 | 合并 |
|------|------------|------|
| Build | `graph/*.json`, `node_defaults/<type>.json`, `init_params` | 浅合并 |
| Runtime | `PipelineRuntimeConfig` | 按 `node_id` 调用 `Operator.configure` |

## 目录映射（对照 PM-Pipeline）

| PM-Pipeline | simple_pipe (C++) |
|-------------|-------------------|
| `pipeline/common/pipeline_graph_spec.h` | `include/simple_pipe/spec/graph_spec.hpp` |
| `pipeline/common/pipeline_builder.h` | `include/simple_pipe/builder/pipeline_builder.hpp` |
| `nodes/core/pm_node.h` | `include/simple_pipe/operators/operator.hpp` |
| `objects/pm_frame_meta.h` | `include/simple_pipe/context/frame_meta.hpp` |
| `SafetyPipeline` / `QualityPipeline` | `SimplePipeline`（可子类加领域校验） |

## 后续可扩展

- `QualityPipeline` 子类：`push_frame(..., program_code)` + 帧级策略节点
- 自定义 `TopologyValidator` 注册到 `SimplePipeline.set_topology_validator`
- C++ 移植：保持 `IGraphSpec` / `IDomainPipeline` / `IOperator` 契约不变
