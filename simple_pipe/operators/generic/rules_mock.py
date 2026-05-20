from __future__ import annotations

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import NodeRole
from simple_pipe.operators.base import Operator


class MockSafetyRules(Operator):
    def __init__(self, node_id: str, build_init_params: dict | None = None) -> None:
        super().__init__(node_id, "mock_safety_rules", build_init_params)
        self._min_score = float(self.build_init_params.get("min_score", 0.5))

    def node_role(self) -> NodeRole:
        return NodeRole.MID

    def configure(self, patch: dict):
        from simple_pipe.result import Result

        if "min_score" in patch:
            self._min_score = float(patch["min_score"])
        return Result.success()

    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        out = meta.copy_shallow()
        dets = out.detections or []
        alerts = [d for d in dets if float(d.get("score", 0)) >= self._min_score]
        out.extensions["safety"] = {"alert": len(alerts) > 0, "alerts": alerts}
        return out
