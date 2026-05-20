from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable

from simple_pipe.enums import DropPolicy, NodeRole
from simple_pipe.operators.base import Operator
from simple_pipe.operators.generic.app_des import AppSink
from simple_pipe.operators.generic.app_src import AppSource
from simple_pipe.operators.registry import NodeFactory, default_node_factory
from simple_pipe.result import Result
from simple_pipe.spec.graph_spec import PipelineGraphSpec


@dataclass
class FlowChartNodeSpec:
    id: str
    type: str
    init_params: dict = field(default_factory=dict)
    upstream_ids: list[str] = field(default_factory=list)


@dataclass
class FlowChartPipelineSpec:
    nodes: list[FlowChartNodeSpec]

    def by_id(self, node_id: str) -> FlowChartNodeSpec | None:
        for node in self.nodes:
            if node.id == node_id:
                return node
        return None


@dataclass
class BuiltPipeline:
    source: AppSource
    sink: AppSink
    nodes: list[Operator]
    node_map: dict[str, Operator]


TopologyValidator = Callable[[FlowChartPipelineSpec], Result[None]]


def _shallow_merge(base: dict, overlay: dict) -> dict:
    merged = dict(base)
    merged.update(overlay)
    return merged


def load_node_defaults(defaults_dir: str | Path, node_type: str) -> dict:
    path = Path(defaults_dir) / f"{node_type}.json"
    if not path.exists():
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


def build_flow_chart_pipeline_spec_from_graph(
    graph: PipelineGraphSpec,
    load_defaults: Callable[[str], dict] | None = None,
) -> FlowChartPipelineSpec:
    load_defaults = load_defaults or (lambda _t: {})
    upstream_map: dict[str, list[str]] = {n.id: [] for n in graph.nodes}
    for edge in graph.edges:
        if edge.to not in upstream_map:
            upstream_map[edge.to] = []
        upstream_map[edge.to].extend(edge.from_nodes)

    flow_nodes: list[FlowChartNodeSpec] = []
    for node in graph.nodes:
        defaults = load_defaults(node.type)
        init_params = _shallow_merge(defaults, node.init_params)
        flow_nodes.append(
            FlowChartNodeSpec(
                id=node.id,
                type=node.type,
                init_params=init_params,
                upstream_ids=upstream_map.get(node.id, []),
            )
        )
    return FlowChartPipelineSpec(nodes=flow_nodes)


def validate_flow_chart_pipeline_spec(
    spec: FlowChartPipelineSpec,
    domain_rules: TopologyValidator | None = None,
) -> Result[None]:
    ids = {n.id for n in spec.nodes}
    if len(ids) != len(spec.nodes):
        return Result.failure("Duplicate node id in graph")

    for node in spec.nodes:
        for upstream in node.upstream_ids:
            if upstream not in ids:
                return Result.failure(f"Edge references unknown upstream: {upstream}")

    roles = {}
    for instance in spec.nodes:
        # role determined after instantiation; validate structural rules on types
        if instance.type == "app_src" and instance.upstream_ids:
            return Result.failure("app_src must not have upstream edges")
        if instance.type == "app_des":
            pass  # may have upstreams only

    sources = [n for n in spec.nodes if n.type == "app_src"]
    sinks = [n for n in spec.nodes if n.type == "app_des"]
    if len(sources) != 1:
        return Result.failure("Graph must contain exactly one app_src")
    if len(sinks) != 1:
        return Result.failure("Graph must contain exactly one app_des")

    if domain_rules:
        return domain_rules(spec)
    return Result.success()


def build_flow_chart_pipeline_from_spec(
    spec: FlowChartPipelineSpec,
    node_factory: NodeFactory | None = None,
) -> BuiltPipeline:
    factory = node_factory or default_node_factory()
    node_map: dict[str, Operator] = {}

    for node_spec in spec.nodes:
        op = factory(node_spec.id, node_spec.type, node_spec.init_params)
        node_map[node_spec.id] = op

    for node_spec in spec.nodes:
        op = node_map[node_spec.id]
        upstreams = [node_map[uid] for uid in node_spec.upstream_ids]
        op.attach_to(upstreams)
        for upstream in upstreams:
            upstream.add_downstream(op)

    sources = [n for n in node_map.values() if n.node_role() == NodeRole.SRC]
    sinks = [n for n in node_map.values() if n.node_role() == NodeRole.DES]
    if len(sources) != 1 or len(sinks) != 1:
        raise RuntimeError("Built graph must have exactly one source and one sink")
    source = sources[0]
    sink = sinks[0]
    if not isinstance(source, AppSource) or not isinstance(sink, AppSink):
        raise RuntimeError("Source/sink type mismatch")

    return BuiltPipeline(
        source=source,
        sink=sink,
        nodes=list(node_map.values()),
        node_map=node_map,
    )


def apply_pipeline_queue_limits(
    built: BuiltPipeline,
    pipeline_params: dict,
    drop_policy: DropPolicy = DropPolicy.DROP_NEWEST,
) -> None:
    src_limit = int(
        pipeline_params.get("app_src_in_queue_limit")
        or pipeline_params.get("source_queue_limit")
        or 8
    )
    des_limit = int(
        pipeline_params.get("app_des_in_queue_limit")
        or pipeline_params.get("sink_queue_limit")
        or 8
    )
    built.source.set_input_queue_limit(src_limit, drop_policy)
    built.sink.set_input_queue_limit(des_limit, drop_policy)
