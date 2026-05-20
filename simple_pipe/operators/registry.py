from __future__ import annotations

from typing import Callable

from simple_pipe.operators.base import Operator
from simple_pipe.operators.generic.app_des import AppSink
from simple_pipe.operators.generic.app_src import AppSource
from simple_pipe.operators.generic.detect_mock import MockDetector
from simple_pipe.operators.generic.motion_mock import MockMotion
from simple_pipe.operators.generic.passthrough import Passthrough
from simple_pipe.operators.generic.resize import Resize
from simple_pipe.operators.generic.rules_mock import MockSafetyRules
from simple_pipe.operators.generic.tracker_mock import MockTracker

NodeFactory = Callable[[str, str, dict], Operator]

_BUILTIN: dict[str, NodeFactory] = {
    "app_src": lambda nid, _, params: AppSource(nid, params),
    "app_des": lambda nid, _, params: AppSink(nid, params),
    "passthrough": lambda nid, _, params: Passthrough(nid, params),
    "resize": lambda nid, _, params: Resize(nid, params),
    "mock_detector": lambda nid, _, params: MockDetector(nid, params),
    "mock_motion": lambda nid, _, params: MockMotion(nid, params),
    "mock_tracker": lambda nid, _, params: MockTracker(nid, params),
    "mock_safety_rules": lambda nid, _, params: MockSafetyRules(nid, params),
}


def default_node_factory() -> NodeFactory:
    registry = dict(_BUILTIN)

    def create(node_id: str, node_type: str, init_params: dict) -> Operator:
        factory = registry.get(node_type)
        if factory is None:
            raise KeyError(f"Unknown node type: {node_type}")
        return factory(node_id, node_type, init_params)

    def register(node_type: str, factory: NodeFactory) -> None:
        registry[node_type] = factory

    create.register = register  # type: ignore[attr-defined]
    return create
