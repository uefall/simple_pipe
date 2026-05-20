from __future__ import annotations

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class AppSource(Operator):
    """Ingress node: frames are injected via PipelineBase.push_frame."""

    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "app_src", build_init_params)

    def node_role(self) -> NodeRole:
        return NodeRole.SRC

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        return meta

    def inject(self, meta: FrameMeta) -> None:
        """Called by pipeline shell instead of meta_flow from upstream."""
        if not self._running:
            return
        out = self.handle_frame_meta(meta)
        if out is not None:
            self._out_queue.put(out)
