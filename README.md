# simple_pipe

**CSPF（Configurable Stream Processing Framework）** 的轻量实现，抽象自 PM-Pipeline 架构原型：声明式 DAG、异步算子节点、`FrameMeta` 帧级载体、Build / Runtime 双平面配置。

## 架构分层

| 层 | 模块 | 说明 |
|----|------|------|
| L0 集成 | `samples/` | 推帧、收回调 |
| L1 门面 | `simple_pipe/pipeline/simple_pipeline.py` | `Build` / `Configure` / `Start` / `PushFrame` |
| L2 运行时壳 | `pipeline_base.py` | 会话状态机 |
| L3 装配 | `builder/pipeline_builder.py` | GraphSpec → 可执行节点图 |
| L4 算子 | `operators/` | `Operator` 基类 + 通用/模拟节点 |
| L5 载体 | `context/frame_meta.py` | `FrameMeta` / `FrameInput` |
| L6 配置 | `configs/<domain>/` | graph JSON + node_defaults |

## 快速开始

```bash
python3 -m venv .venv && source .venv/bin/activate
pip install -e ".[dev]"
pytest
python samples/run_demo.py
```

## 最小集成序列

```python
from simple_pipe.context.frame_meta import FrameInput
from simple_pipe.pipeline.simple_pipeline import SimplePipeline
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig

pipe = SimplePipeline(defaults_dir="configs/demo/node_defaults")
pipe.build_from_json("configs/demo/graph/linear.json")
pipe.configure(PipelineRuntimeConfig.load("configs/demo/runtime.example.json"))
pipe.set_output_callback(lambda meta: print(meta.extensions))
pipe.start()
pipe.push_frame(FrameInput(image="data", frame_index=0))
pipe.stop()
```

## 扩展新算子

1. 继承 `Operator`，实现 `node_role()` 与 `handle_frame_meta()`。
2. 在 `operators/registry.py` 注册 `type` 字符串。
3. 在 `configs/<domain>/graph/*.json` 中引用该 `type`。

## 示例拓扑

- **线性**：`configs/demo/graph/linear.json` — `app_src → resize → app_des`
- **Safety 风格 fan-in**：`configs/demo/graph/safety_fanin.json` — 检测与运动并行后汇入 tracker

## 文档

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — 与 PM-Pipeline / CSPF 原型的概念对照

## 许可证

MIT（示例代码）
