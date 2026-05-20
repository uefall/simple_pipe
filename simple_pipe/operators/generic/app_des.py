from __future__ import annotations

from typing import Callable

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class AppSink(Operator):
    """Egress node: invokes pipeline output callback."""

    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "app_des", build_init_params)
        self._output_callback: Callable[[FrameMeta], None] | None = None

    def node_role(self) -> NodeRole:
        return NodeRole.DES

    def set_output_callback(self, cb: Callable[[FrameMeta], None]) -> None:
        self._output_callback = cb

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        if self._output_callback:
            self._output_callback(meta)
        return None
