from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class PipelineRuntimeConfig:
    pipeline_params: dict[str, Any] = field(default_factory=dict)
    node_params: dict[str, dict[str, Any]] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> PipelineRuntimeConfig:
        node_params = dict(data.get("node_params") or data.get("nodes") or {})
        return cls(
            pipeline_params=dict(data.get("pipeline_params") or {}),
            node_params={k: dict(v) if isinstance(v, dict) else {} for k, v in node_params.items()},
        )

    @classmethod
    def from_json(cls, text: str) -> PipelineRuntimeConfig:
        return cls.from_dict(json.loads(text))

    @classmethod
    def load(cls, path: str | Path) -> PipelineRuntimeConfig:
        return cls.from_json(Path(path).read_text(encoding="utf-8"))
