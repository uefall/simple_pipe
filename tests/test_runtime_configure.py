import threading
from pathlib import Path

from simple_pipe.context.frame_meta import FrameInput
from simple_pipe.pipeline.simple_pipeline import SimplePipeline
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig

ROOT = Path(__file__).resolve().parents[1]


def test_configure_while_running_keeps_pipeline_active():
    pipeline = SimplePipeline()
    assert pipeline.build_from_json(ROOT / "configs/demo/graph/five_node.json").ok
    pipeline.configure(PipelineRuntimeConfig.from_dict({"node_params": {"rules": {"min_score": 0.5}}}))

    done = threading.Event()
    pipeline.set_output_callback(lambda _: done.set())
    assert pipeline.start().ok

    assert pipeline.push_frame(FrameInput(image="a", frame_index=0)).ok
    assert pipeline.configure(
        PipelineRuntimeConfig.from_dict({"node_params": {"rules": {"min_score": 0.99}}})
    ).ok
    assert pipeline.is_running()
    assert pipeline.push_frame(FrameInput(image="b", frame_index=1)).ok

    assert done.wait(timeout=3.0)
    pipeline.stop()
