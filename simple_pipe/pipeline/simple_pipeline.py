from __future__ import annotations

from pathlib import Path

from simple_pipe.builder.pipeline_builder import TopologyValidator, load_node_defaults
from simple_pipe.pipeline.pipeline_base import PipelineBase
from simple_pipe.result import Result
from simple_pipe.spec.graph_spec import PipelineGraphSpec, load_graph_spec_from_json


def _default_topology_validator(spec) -> Result[None]:
    """Allow fan-in for tracker-like nodes; ensure linear reachability is host responsibility."""
    return Result.success()


class SimplePipeline(PipelineBase):
    """
    Generic domain pipeline facade — maps PM-Pipeline CSPF to a minimal Python runtime.
    """

    def __init__(self, defaults_dir: str | Path | None = None) -> None:
        super().__init__()
        self._defaults_dir = Path(defaults_dir) if defaults_dir else None
        self._domain_validator: TopologyValidator | None = None

    def domain_name(self) -> str:
        return "simple"

    def set_topology_validator(self, validator: TopologyValidator) -> None:
        self._domain_validator = validator

    def build_from_json(self, graph_path: str | Path) -> Result[None]:
        graph = load_graph_spec_from_json(graph_path)
        load_defaults = None
        if self._defaults_dir:
            load_defaults = lambda t: load_node_defaults(self._defaults_dir, t)
        validator = self._domain_validator or _default_topology_validator
        return self.build(graph, load_defaults=load_defaults, domain_validator=validator)

    def build_graph(self, graph: PipelineGraphSpec) -> Result[None]:
        load_defaults = None
        if self._defaults_dir:
            load_defaults = lambda t: load_node_defaults(self._defaults_dir, t)
        validator = self._domain_validator or _default_topology_validator
        return self.build(graph, load_defaults=load_defaults, domain_validator=validator)
