# 构建与环境说明

## 环境总览

| 组件 | 版本要求 | Ubuntu | macOS | 获取方式 |
|------|----------|--------|-------|----------|
| CMake | ≥ 3.16 | `apt: cmake` | `brew: cmake` | 系统包 |
| C++ 编译器 | C++17 | `g++`（推荐） | `clang++`（Xcode CLT） | 系统包 |
| Git | 任意较新 | `apt: git` | Xcode CLT / brew | 配置 FetchContent |
| nlohmann/json | 3.11.x | — | — | **自动** FetchContent |
| GoogleTest | 1.14.x | — | — | **自动** FetchContent（`BUILD_TESTS=ON`） |
| OpenCV | ≥ 4.x（可选） | `libopencv-dev` | `brew install opencv` | 系统包；关闭后无 `image_src`/`video_src` |

**不包含**：PMSdk、厂商 NPU 运行时、Python 运行时（C++ 主路径）。

## 平台支持矩阵

| 能力 | Ubuntu 20.04+ / Debian | macOS 12+ (Intel/Apple Silicon) |
|------|------------------------|----------------------------------|
| 核心库 `simple_pipe` | ✅ | ✅ |
| GTest | ✅ | ✅ |
| `app_src` + mock 节点 | ✅ | ✅ |
| `image_src` / `video_src` | ✅（需 OpenCV） | ✅（需 `brew install opencv`） |
| 示例程序 | ✅ | ✅ |

## 推荐：统一构建脚本

```bash
chmod +x scripts/build.sh

# Ubuntu：安装依赖 + Release + 跑测试
./scripts/build.sh --install-deps --test

# macOS：Homebrew 依赖 + 测试
./scripts/build.sh --platform macos --install-deps --test

# 无 OpenCV 最小构建（适合无 GUI/无媒体环境）
./scripts/build.sh --with-opencv off --test

# Debug + 指定编译器
./scripts/build.sh --build-type Debug --compiler clang++ -j 8 --test
```

### 常用选项

| 选项 | 说明 |
|------|------|
| `--platform auto\|ubuntu\|macos` | 平台提示（依赖安装、OpenCV 路径） |
| `--with-opencv auto\|on\|off` | 是否编译媒体 Reader |
| `--install-deps` | Ubuntu 用 `apt`；macOS 用 `brew` |
| `--opencv-dir <path>` | 手动指定 `OpenCV_DIR`（mac 多版本时） |
| `--cmake-arg -D...` | 透传额外 CMake 参数 |
| `--clean` | 删除 build 目录后重配 |
| `--test` | 构建后执行 `ctest` |

## 手动 CMake（不通过脚本）

### Ubuntu

```bash
sudo apt-get install -y build-essential cmake libopencv-dev
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

### macOS

```bash
xcode-select --install   # 若尚未安装命令行工具
brew install cmake opencv
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix)" \
  -DOpenCV_DIR="$(brew --prefix opencv)/lib/cmake/opencv4"
cmake --build build -j"$(sysctl -n hw.ncpu)"
ctest --test-dir build --output-on-failure
```

## CMake 选项一览

| CMake 选项 | 默认 | 说明 |
|------------|------|------|
| `SIMPLE_PIPE_WITH_OPENCV` | ON | 关闭后不编译 `image_src`/`video_src` |
| `SIMPLE_PIPE_BUILD_TESTS` | ON | GTest 与 `simple_pipe_tests` |
| `SIMPLE_PIPE_BUILD_SAMPLES` | ON | `run_demo_cpp`、`run_five_node_100_cpp` |

## 常见问题

### Ubuntu：`cannot find -lstdc++`（Clang 默认）

使用 g++ 或安装 `libstdc++-12-dev`：

```bash
./scripts/build.sh --compiler g++
```

### macOS：找不到 OpenCV

```bash
brew install opencv
./scripts/build.sh --platform macos --opencv-dir "$(brew --prefix opencv)/lib/cmake/opencv4"
```

### 不需要 OpenCV

```bash
./scripts/build.sh --with-opencv off
```

### Apple Silicon

与 Intel macOS 相同；Homebrew 前缀在 `/opt/homebrew`，脚本会通过 `brew --prefix` 自动处理。
