from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class GraphNodeSpec:
    id: str
    type: str
    init_params: dict[str, Any] = field(default_factory=dict)


@dataclass
class GraphEdgeSpec:
    to: str
    from_nodes: list[str] = field(default_factory=list)


@dataclass
class PipelineGraphSpec:
    nodes: list[GraphNodeSpec]
    edges: list[GraphEdgeSpec]

    def node_ids(self) -> set[str]:
        return {n.id for n in self.nodes}

    def node_by_id(self, node_id: str) -> GraphNodeSpec | None:
        for node in self.nodes:
            if node.id == node_id:
                return node
        return None


def _parse_node(raw: dict[str, Any]) -> GraphNodeSpec:
    return GraphNodeSpec(
        id=raw["id"],
        type=raw["type"],
        init_params=dict(raw.get("init_params") or raw.get("initParams") or {}),
    )


def _parse_edge(raw: dict[str, Any]) -> GraphEdgeSpec:
    upstream = raw.get("from") or raw.get("from_nodes") or []
    if isinstance(upstream, str):
        upstream = [upstream]
    return GraphEdgeSpec(to=raw["to"], from_nodes=list(upstream))


def pipeline_graph_spec_from_dict(data: dict[str, Any]) -> PipelineGraphSpec:
    nodes = [_parse_node(n) for n in data.get("nodes", [])]
    edges = [_parse_edge(e) for e in data.get("edges", [])]
    return PipelineGraphSpec(nodes=nodes, edges=edges)


def load_graph_spec_from_json(path: str | Path) -> PipelineGraphSpec:
    text = Path(path).read_text(encoding="utf-8")
    return pipeline_graph_spec_from_dict(json.loads(text))
