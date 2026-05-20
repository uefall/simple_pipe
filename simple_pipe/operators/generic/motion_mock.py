from __future__ import annotations

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class MockMotion(Operator):
    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "mock_motion", build_init_params)

    def node_role(self) -> NodeRole:
        return NodeRole.MID

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        out = meta.copy_shallow()
        out.extensions["motion"] = {"moving": meta.frame_index % 2 == 0}
        return out
