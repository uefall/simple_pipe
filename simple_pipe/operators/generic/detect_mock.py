from __future__ import annotations

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class MockDetector(Operator):
    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "mock_detector", build_init_params)
        self._label = str(self.build_init_params.get("label", "object"))

    def node_role(self) -> NodeRole:
        return NodeRole.MID

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        out = meta.copy_shallow()
        out.detections = (out.detections or []) + [
            {"source": self.node_id, "label": self._label, "score": 0.9}
        ]
        return out
