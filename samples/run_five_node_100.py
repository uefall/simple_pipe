#!/usr/bin/env python3
"""
五节点管线示例：模拟 100 帧输入，并在运行中动态 Configure 部分节点。

拓扑（5 节点）:
  app_src → resize → detector → rules → app_des

生命周期:
  1. Build(graph)
  2. Configure(初始 runtime)
  3. Start()
  4. PushFrame × 100（第 50 帧后热更新 resize / rules / 队列深度）
  5. Stop()
"""

from __future__ import annotations

import sys
import threading
import time
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from simple_pipe.context.frame_meta import FrameInput
from simple_pipe.pipeline.simple_pipeline import SimplePipeline
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig

GRAPH = ROOT / "configs" / "demo" / "graph" / "five_node.json"
DEFAULTS_DIR = ROOT / "configs" / "demo" / "node_defaults"
FRAME_COUNT = 100
RUNTIME_PATCH_AT_FRAME = 50
# 推帧间隔（秒），避免瞬时打满队列导致大量 DROP_NEWEST 丢帧
PUSH_INTERVAL_S = 0.002


def main() -> None:
    pipeline = SimplePipeline(defaults_dir=DEFAULTS_DIR)

    build = pipeline.build_from_json(GRAPH)
    if not build.ok:
        raise RuntimeError(f"Build failed: {build.error}")

    # 初始运行时配置（Build 之后、Start 之前）
    initial_runtime = PipelineRuntimeConfig.from_dict(
        {
            "pipeline_params": {
                "app_src_in_queue_limit": 16,
                "app_des_in_queue_limit": 16,
            },
            "node_params": {
                "resize": {"width": 800, "height": 600},
                "rules": {"min_score": 0.5},
            },
        }
    )
    cfg = pipeline.configure(initial_runtime)
    if not cfg.ok:
        raise RuntimeError(f"Configure failed: {cfg.error}")

    received = 0
    alerts_before_patch = 0
    alerts_after_patch = 0
    resize_width_hist: Counter[int] = Counter()
    lock = threading.Lock()
    done = threading.Event()

    def on_output(meta) -> None:
        nonlocal received, alerts_before_patch, alerts_after_patch
        safety = meta.extensions.get("safety") or {}
        alert = bool(safety.get("alert"))
        resize_w = (meta.extensions.get("resize") or {}).get("width")
        with lock:
            received += 1
            if meta.frame_index < RUNTIME_PATCH_AT_FRAME:
                if alert:
                    alerts_before_patch += 1
            else:
                if alert:
                    alerts_after_patch += 1
            if resize_w is not None:
                resize_width_hist[resize_w] += 1
            if received >= FRAME_COUNT:
                done.set()

    pipeline.set_output_callback(on_output)
    if not pipeline.start().ok:
        raise RuntimeError("Start failed")

    print("Pipeline running — 5 nodes, pushing 100 frames …")
    print(f"  graph: {GRAPH.relative_to(ROOT)}")
    print(f"  runtime patch at frame index >= {RUNTIME_PATCH_AT_FRAME}")

    t0 = time.perf_counter()
    for i in range(FRAME_COUNT):
        if i == RUNTIME_PATCH_AT_FRAME:
            # 运行中热更新：改 resize 尺寸、提高 rules 阈值（节点级 patch）
            patch_runtime = PipelineRuntimeConfig.from_dict(
                {
                    "node_params": {
                        "resize": {"width": 1920, "height": 1080},
                        "rules": {"min_score": 0.95},
                    },
                }
            )
            mid = pipeline.configure(patch_runtime)
            if not mid.ok:
                pipeline.stop()
                raise RuntimeError(f"Mid-run Configure failed: {mid.error}")
            print(f"  [frame {i}] runtime Configure applied (resize=1920x1080, min_score=0.95)")

        result = pipeline.push_frame(
            FrameInput(
                image=f"mock-image-{i}",
                frame_index=i,
                channel_index=0,
            )
        )
        if not result.ok:
            pipeline.stop()
            raise RuntimeError(f"PushFrame failed at {i}: {result.error}")
        time.sleep(PUSH_INTERVAL_S)

    if not done.wait(timeout=30.0):
        pipeline.stop()
        raise TimeoutError(f"Only received {received}/{FRAME_COUNT} sink callbacks")

    elapsed = time.perf_counter() - t0
    pipeline.stop()

    print()
    print("=== Summary ===")
    print(f"  frames pushed:     {FRAME_COUNT}  (PushFrame × {FRAME_COUNT})")
    print(f"  sink callbacks:    {received}")
    print(f"  elapsed:           {elapsed:.3f}s ({FRAME_COUNT / elapsed:.1f} fps push)")
    print(f"  alerts [0, {RUNTIME_PATCH_AT_FRAME}):   {alerts_before_patch}")
    print(f"  alerts [{RUNTIME_PATCH_AT_FRAME}, 100): {alerts_after_patch}")
    print(f"  resize width hist: {dict(sorted(resize_width_hist.items()))}")
    print()
    print("Expected: before patch most frames alert (min_score=0.5);")
    print("          after patch fewer alerts (min_score=0.95); resize width shifts to 1920.")


if __name__ == "__main__":
    main()
