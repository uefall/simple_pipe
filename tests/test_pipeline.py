import threading
import time
from pathlib import Path

from simple_pipe.context.frame_meta import FrameInput
from simple_pipe.pipeline.simple_pipeline import SimplePipeline
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig

ROOT = Path(__file__).resolve().parents[1]


def _run_frames(pipeline: SimplePipeline, count: int = 2, timeout: float = 5.0):
    results = []
    done = threading.Event()

    def on_output(meta):
        results.append(meta)
        if len(results) >= count:
            done.set()

    pipeline.set_output_callback(on_output)
    assert pipeline.start().ok
    for i in range(count):
        assert pipeline.push_frame(FrameInput(image="x", frame_index=i)).ok
    assert done.wait(timeout), "timed out waiting for sink callback"
    pipeline.stop()
    return results


def test_linear_pipeline():
    pipeline = SimplePipeline(defaults_dir=ROOT / "configs/demo/node_defaults")
    assert pipeline.build_from_json(ROOT / "configs/demo/graph/linear.json").ok
    results = _run_frames(pipeline, count=2)
    assert results[0].extensions.get("resize", {}).get("width") == 320


def test_safety_fanin_pipeline():
    pipeline = SimplePipeline()
    assert pipeline.build_from_json(ROOT / "configs/demo/graph/safety_fanin.json").ok
    pipeline.configure(PipelineRuntimeConfig.from_dict({"node_params": {"rules": {"min_score": 0.85}}}))
    results = _run_frames(pipeline, count=2)
    assert "safety" in results[-1].extensions
    assert "tracks" in results[-1].extensions
