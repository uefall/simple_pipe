# CSPF 架构说明（simple_pipe）

`simple_pipe` 是 **standalone** 的可配置流处理框架（C++17），不绑定任何厂商 SDK。

## 与 PM-Pipeline 文档的关系

早期讨论中的 **PM-Pipeline / PMSdk / 许可初始化** 等属于另一套产品化方案的描述，**未纳入本仓库**。此处仅保留概念对照（Facade、DAG、双平面配置），便于从设计文档迁移思路，**不是运行时依赖**。

## 依赖边界

| 包含 | 不包含 |
|------|--------|
| nlohmann/json（FetchContent） | PMSdk、商业许可网关 |
| GTest（测试） | Sophon / 厂商 NPU 运行时 |
| OpenCV（可选，`image_src`/`video_src`） | 业务算法 SO、预编译 pmind 等 |

## 核心范式

- **Pipeline-and-Filter**：节点链式处理 `FrameMeta`
- **DAG**：`PipelineGraphSpec.edges`
- **Observer**：`meta_flow` 推送
- **Facade**：`SimplePipeline`

## Ingress（帧入口）

```text
Push:  Host → PushFrame → app_src → …
Pull:  image_src / video_src 内部 Reader 线程 → Inject → …
```

拓扑规则：**恰好一个** `app_src` | `image_src` | `video_src`。

## 媒体层（OpenCV 第一版）

```text
IMediaReader
  ├── OpenCvImageReader   (path / paths / 目录)
  └── OpenCvVideoReader   (uri / device, target_fps)

PullSource → ImageSource / VideoSource
FrameMeta.buffer → shared_ptr<FrameBuffer>（cv::Mat）
```

## 日志模块

`simple_pipe::log::Logger`：线程安全、分级（Debug/Info/Warn/Error）、输出到 stdout/stderr。

## 会话状态机

```text
Unbuilt → Build → Configure → StartPipeline → Running → StopPipeline
```

Pull 模式下 **禁止** `PushFrame`（由 builder 标记 `pull_driven`）。

## 配置双平面

| 平面 | 内容 |
|------|------|
| Build | graph JSON + node_defaults + init_params |
| Runtime | pipeline_params + node_params（可运行中 patch） |
