# simple_pipe

**CSPF（Configurable Stream Processing Framework）** — 独立的 C++17 流处理框架：声明式 DAG、异步算子、`FrameMeta` 帧载体、Build / Runtime 双平面配置。

> **说明**：本仓库是通用框架实现，**不包含** PM-Pipeline / PMSdk / Sophon 等商业或旧架构中的第三方 SDK。概念上借鉴了「声明式图 + 节点数据流」思想，但依赖与代码均为 standalone。

## 贡献

**禁止直接向 `main` 推送。** 请从 `main` 拉取 `cursor/` 前缀分支，通过 Pull Request 合并。详见 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 依赖

| 依赖 | 用途 | 获取方式 |
|------|------|----------|
| C++17 | 语言 | 系统工具链 |
| [nlohmann/json](https://github.com/nlohmann/json) | Graph / Runtime JSON | CMake FetchContent |
| [GoogleTest](https://github.com/google/googletest) | 单元测试 | CMake FetchContent（可选） |
| [OpenCV](https://opencv.org/) | `image_src` / `video_src` Reader | 系统包 `libopencv-dev`（可选，`SIMPLE_PIPE_WITH_OPENCV=ON`） |

## 构建（Ubuntu / macOS）

推荐使用跨平台脚本（详见 [docs/BUILD.md](docs/BUILD.md)）：

```bash
chmod +x scripts/build.sh

# Ubuntu
./scripts/build.sh --install-deps --test

# macOS (Homebrew)
./scripts/build.sh --platform macos --install-deps --test

# 无 OpenCV 最小构建
./scripts/build.sh --with-opencv off --test
```

手动 CMake 示例：

```bash
# Ubuntu
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build -j && ctest --test-dir build

# macOS
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_PREFIX_PATH="$(brew --prefix)" \
  -DOpenCV_DIR="$(brew --prefix opencv)/lib/cmake/opencv4"
cmake --build build -j && ctest --test-dir build
```

## Ingress 节点（帧入口，三选一）

| type | 模式 | 说明 |
|------|------|------|
| `app_src` | Push | Host 调用 `PushFrame()` |
| `image_src` | Pull | OpenCV 读图片/目录，内部线程 `Inject` |
| `video_src` | Pull | OpenCV `VideoCapture` 读文件/设备 |

## 日志

```cpp
#include "simple_pipe/log/logger.hpp"

simple_pipe::log::Logger::Instance().SetLevel(simple_pipe::log::Level::kDebug);
simple_pipe::log::Info("MyNode", "started");
```

## 运行示例

```bash
./build/samples/run_demo_cpp
./build/samples/run_five_node_100_cpp
```

## 示例 Graph

| 文件 | 拓扑 |
|------|------|
| `linear.json` | app_src → resize → app_des |
| `five_node.json` | app_src → resize → detector → rules → app_des |
| `safety_fanin.json` | 检测 ∥ 运动 → tracker → rules → sink |
| `image_src_linear.json` | image_src → resize → app_des（需 OpenCV） |

## 目录结构

```text
include/simple_pipe/     # 头文件（log / media / operators / pipeline）
src/
configs/demo/graph/
tests/
samples/
docs/ARCHITECTURE.md
```

## 扩展算子

1. 继承 `Operator`（或 `PullSource` / `SourceOperator`）
2. 在 `src/operators/node_registry.cpp` 注册 `type`
3. 在 `configs/.../graph/*.json` 引用

Python 目录 `simple_pipe/*.py` 为早期参考实现，**主构建以 C++ 为准**。
