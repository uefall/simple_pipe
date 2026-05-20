from pathlib import Path

import pytest

from simple_pipe.builder.pipeline_builder import (
    build_flow_chart_pipeline_spec_from_graph,
    validate_flow_chart_pipeline_spec,
)
from simple_pipe.spec.graph_spec import load_graph_spec_from_json

ROOT = Path(__file__).resolve().parents[1]


def test_load_linear_graph():
    graph = load_graph_spec_from_json(ROOT / "configs/demo/graph/linear.json")
    flow = build_flow_chart_pipeline_spec_from_graph(graph)
    result = validate_flow_chart_pipeline_spec(flow)
    assert result.ok
    assert len(flow.nodes) == 3


def test_validate_rejects_duplicate_src():
    graph = load_graph_spec_from_json(ROOT / "configs/demo/graph/linear.json")
    graph.nodes.append(graph.nodes[0].__class__(id="app_src2", type="app_src"))
    flow = build_flow_chart_pipeline_spec_from_graph(graph)
    result = validate_flow_chart_pipeline_spec(flow)
    assert not result.ok
