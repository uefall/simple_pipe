from __future__ import annotations

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class Resize(Operator):
    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "resize", build_init_params)
        self._width = int(self.build_init_params.get("width", 640))
        self._height = int(self.build_init_params.get("height", 480))

    def node_role(self) -> NodeRole:
        return NodeRole.MID

    def configure(self, patch: dict):
        from simple_pipe.result import Result

        if "width" in patch:
            self._width = int(patch["width"])
        if "height" in patch:
            self._height = int(patch["height"])
        return Result.success()

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        out = meta.copy_shallow()
        out.extensions["resize"] = {"width": self._width, "height": self._height}
        return out
