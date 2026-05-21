#!/usr/bin/env bash
# simple_pipe cross-platform build helper (Ubuntu / macOS)
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
BUILD_TYPE="Release"
PLATFORM="auto"
WITH_OPENCV="auto"
BUILD_TESTS="ON"
BUILD_SAMPLES="ON"
INSTALL_DEPS=0
RUN_TESTS=0
CLEAN=0
JOBS=""
CXX_COMPILER=""
OPENCV_DIR=""
CMAKE_EXTRA=()

usage() {
  cat <<'EOF'
Usage: scripts/build.sh [options]

Cross-platform build for simple_pipe (Ubuntu / macOS).

Options:
  --platform <auto|ubuntu|macos>   Target platform hint (default: auto-detect)
  --build-type <Release|Debug>     CMake build type (default: Release)
  --build-dir <path>               CMake build directory (default: ./build)
  --with-opencv <auto|on|off>      Enable OpenCV readers (default: auto=on)
  --no-tests                       Disable GTest build
  --no-samples                     Disable sample binaries
  --install-deps                   Install system packages (apt/brew)
  --compiler <g++|clang++|path>    C++ compiler for CMake
  --opencv-dir <path>              OpenCV_DIR for CMake (macOS Homebrew, etc.)
  --cmake-arg <ARG>                Extra CMake configure argument (repeatable)
  -j, --jobs <N>                   Parallel build jobs
  --clean                          Remove build directory before configure
  --test                           Run ctest after build
  -h, --help                       Show this help

Examples:
  # Ubuntu: install deps + release build + tests
  ./scripts/build.sh --install-deps --test

  # macOS (Homebrew OpenCV)
  ./scripts/build.sh --platform macos --install-deps --test

  # Minimal build without OpenCV (CI / no media)
  ./scripts/build.sh --with-opencv off --test

  # Debug + Clang on macOS
  ./scripts/build.sh --platform macos --build-type Debug --compiler clang++ --test
EOF
}

detect_platform() {
  local uname_s
  uname_s="$(uname -s)"
  case "${uname_s}" in
    Linux*) PLATFORM="ubuntu" ;;
    Darwin*) PLATFORM="macos" ;;
    *) PLATFORM="unknown" ;;
  esac
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --platform)
        PLATFORM="$2"
        shift 2
        ;;
      --build-type)
        BUILD_TYPE="$2"
        shift 2
        ;;
      --build-dir)
        BUILD_DIR="$2"
        shift 2
        ;;
      --with-opencv)
        WITH_OPENCV="$2"
        shift 2
        ;;
      --no-tests)
        BUILD_TESTS="OFF"
        shift
        ;;
      --no-samples)
        BUILD_SAMPLES="OFF"
        shift
        ;;
      --install-deps)
        INSTALL_DEPS=1
        shift
        ;;
      --compiler)
        CXX_COMPILER="$2"
        shift 2
        ;;
      --opencv-dir)
        OPENCV_DIR="$2"
        shift 2
        ;;
      --cmake-arg)
        CMAKE_EXTRA+=("$2")
        shift 2
        ;;
      -j)
        if [[ -n "${2:-}" && "$2" =~ ^[0-9]+$ ]]; then
          JOBS="$2"
          shift 2
        else
          # allow -j8 form
          JOBS="${1#-j}"
          shift
        fi
        ;;
      --jobs)
        JOBS="$2"
        shift 2
        ;;
      --clean)
        CLEAN=1
        shift
        ;;
      --test)
        RUN_TESTS=1
        shift
        ;;
      -h|--help)
        usage
        exit 0
        ;;
      *)
        echo "Unknown option: $1" >&2
        usage >&2
        exit 1
        ;;
    esac
  done
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing required command: $1" >&2
    exit 1
  fi
}

install_deps_ubuntu() {
  echo "==> Installing Ubuntu/Debian dependencies (sudo)..."
  sudo apt-get update
  sudo apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libopencv-dev
}

install_deps_macos() {
  echo "==> Installing macOS dependencies via Homebrew..."
  require_cmd brew
  brew update
  brew install cmake ninja opencv pkg-config
}

resolve_macos_opencv() {
  if [[ -n "${OPENCV_DIR}" ]]; then
    return
  fi
  if command -v brew >/dev/null 2>&1; then
    local prefix
    prefix="$(brew --prefix opencv 2>/dev/null || true)"
    if [[ -n "${prefix}" && -d "${prefix}/lib/cmake/opencv4" ]]; then
      OPENCV_DIR="${prefix}/lib/cmake/opencv4"
    elif [[ -n "${prefix}" && -f "${prefix}/OpenCVConfig.cmake" ]]; then
      OPENCV_DIR="${prefix}"
    fi
  fi
}

resolve_compiler() {
  if [[ -n "${CXX_COMPILER}" ]]; then
    return
  fi
  if [[ "${PLATFORM}" == "macos" ]]; then
    if command -v clang++ >/dev/null 2>&1; then
      CXX_COMPILER="clang++"
    fi
  else
    if command -v g++ >/dev/null 2>&1; then
      CXX_COMPILER="g++"
    elif command -v clang++ >/dev/null 2>&1; then
      CXX_COMPILER="clang++"
    fi
  fi
}

resolve_jobs() {
  if [[ -n "${JOBS}" ]]; then
    return
  fi
  if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
  elif command -v sysctl >/dev/null 2>&1; then
    JOBS="$(sysctl -n hw.ncpu)"
  else
    JOBS="4"
  fi
}

opencv_cmake_flag() {
  case "${WITH_OPENCV}" in
    on|ON|yes|true)
      echo "ON"
      ;;
    off|OFF|no|false)
      echo "OFF"
      ;;
    auto|*)
      echo "ON"
      ;;
  esac
}

main() {
  parse_args "$@"

  if [[ "${PLATFORM}" == "auto" ]]; then
    detect_platform
  fi

  echo "==> simple_pipe build"
  echo "    root:      ${ROOT_DIR}"
  echo "    platform:  ${PLATFORM}"
  echo "    build dir: ${BUILD_DIR}"
  echo "    type:      ${BUILD_TYPE}"

  require_cmd cmake
  require_cmd git

  if [[ "${INSTALL_DEPS}" -eq 1 ]]; then
    case "${PLATFORM}" in
      ubuntu) install_deps_ubuntu ;;
      macos) install_deps_macos ;;
      *)
        echo "Cannot install deps for platform: ${PLATFORM}" >&2
        exit 1
        ;;
    esac
  fi

  resolve_compiler
  resolve_jobs

  if [[ "${PLATFORM}" == "macos" ]]; then
    resolve_macos_opencv
  fi

  if [[ "${CLEAN}" -eq 1 && -d "${BUILD_DIR}" ]]; then
    echo "==> Cleaning ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
  fi

  mkdir -p "${BUILD_DIR}"

  local opencv_flag
  opencv_flag="$(opencv_cmake_flag)"

  local -a cmake_args=(
    -S "${ROOT_DIR}"
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
    -DCMAKE_CXX_STANDARD=17
    -DSIMPLE_PIPE_BUILD_TESTS="${BUILD_TESTS}"
    -DSIMPLE_PIPE_BUILD_SAMPLES="${BUILD_SAMPLES}"
    -DSIMPLE_PIPE_WITH_OPENCV="${opencv_flag}"
  )

  if [[ -n "${CXX_COMPILER}" ]]; then
    cmake_args+=(-DCMAKE_CXX_COMPILER="${CXX_COMPILER}")
  fi

  if [[ "${opencv_flag}" == "ON" && -n "${OPENCV_DIR}" ]]; then
    cmake_args+=(-DOpenCV_DIR="${OPENCV_DIR}")
  fi

  if [[ "${PLATFORM}" == "macos" && "${opencv_flag}" == "ON" ]]; then
    if command -v brew >/dev/null 2>&1; then
      local brew_prefix
      brew_prefix="$(brew --prefix)"
      cmake_args+=(-DCMAKE_PREFIX_PATH="${brew_prefix}")
    fi
  fi

  if [[ ${#CMAKE_EXTRA[@]} -gt 0 ]]; then
    cmake_args+=("${CMAKE_EXTRA[@]}")
  fi

  echo "==> CMake configure: ${cmake_args[*]}"
  cmake "${cmake_args[@]}"

  echo "==> CMake build (-j ${JOBS})"
  cmake --build "${BUILD_DIR}" -j "${JOBS}"

  if [[ "${RUN_TESTS}" -eq 1 ]]; then
    if [[ "${BUILD_TESTS}" != "ON" ]]; then
      echo "Tests disabled; skip --test" >&2
      exit 1
    fi
    echo "==> Running tests"
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
  fi

  echo "==> Done."
  echo "    Binaries: ${BUILD_DIR}/samples/"
  echo "    Tests:    ${BUILD_DIR}/tests/simple_pipe_tests"
}

main "$@"
