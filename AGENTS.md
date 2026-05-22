# AGENTS.md

## Cursor Cloud specific instructions

This is a standalone C++17 stream-processing framework (CSPF) with a Python reference implementation. No databases, Docker, or external services are required.

### Services overview

| Component | Language | Purpose |
|-----------|----------|---------|
| `simple_pipe` (C++ lib) | C++17 | Core framework — DAG pipeline, async operators, FrameMeta |
| Python `simple_pipe` | Python 3.10+ | Reference implementation (secondary) |

### Build & test commands

All standard commands are documented in [docs/BUILD.md](docs/BUILD.md) and [README.md](README.md).

- **C++ full build + test:** `./scripts/build.sh --test`
- **C++ build without OpenCV:** `./scripts/build.sh --with-opencv off --test`
- **C++ tests only (after build):** `ctest --test-dir build --output-on-failure`
- **Python tests:** `pytest tests/ -v`
- **C++ samples:** `./build/samples/run_demo_cpp` and `./build/samples/run_five_node_100_cpp`
- **Python samples:** `python3 samples/run_demo.py` and `python3 samples/run_five_node_100.py`

### Non-obvious caveats

- System dependencies (`build-essential`, `cmake`, `ninja-build`, `libopencv-dev`, `pkg-config`) must be pre-installed via apt. The update script handles this.
- CMake `FetchContent` downloads `nlohmann/json` and `GoogleTest` automatically during configure — first build takes longer due to network fetches; subsequent builds use cached content in `build/_deps/`.
- The `build/` directory is not checked in. If you need a clean rebuild, pass `--clean` to `scripts/build.sh`.
- Python package is installed in editable mode (`pip install -e ".[dev]"`); changes to `simple_pipe/` Python files are picked up immediately.
- `pytest` may be installed to `~/.local/bin`; ensure `PATH` includes it (e.g. `export PATH="$HOME/.local/bin:$PATH"`).
