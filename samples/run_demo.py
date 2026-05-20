#!/usr/bin/env python3
"""Run demo pipelines from configs/demo/graph/*.json"""

from __future__ import annotations

import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from simple_pipe.context.frame_meta import FrameInput
from simple_pipe.pipeline.simple_pipeline import SimplePipeline
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig


def run_graph(name: str) -> None:
    graph_path = ROOT / "configs" / "demo" / "graph" / f"{name}.json"
    defaults_dir = ROOT / "configs" / "demo" / "node_defaults"
    pipeline = SimplePipeline(defaults_dir=defaults_dir)

    results: list = []
    done = threading.Event()

    def on_output(meta):
        results.append(meta)
        if len(results) >= 3:
            done.set()

    build_result = pipeline.build_from_json(graph_path)
    if not build_result.ok:
        raise RuntimeError(build_result.error)

    runtime_path = ROOT / "configs" / "demo" / "runtime.example.json"
    if runtime_path.exists():
        pipeline.configure(PipelineRuntimeConfig.load(runtime_path))

    pipeline.set_output_callback(on_output)
    pipeline.start()

    for i in range(3):
        pipeline.push_frame(FrameInput(image=f"frame-{i}", frame_index=i))

    if not done.wait(timeout=5.0):
        raise TimeoutError(f"No output for graph {name}")

    pipeline.stop()
    print(f"[{name}] frames={len(results)} last_safety={results[-1].extensions.get('safety')}")


if __name__ == "__main__":
    run_graph("linear")
    run_graph("safety_fanin")
