from __future__ import annotations

from abc import ABC, abstractmethod
from enum import Enum
from typing import Callable

from simple_pipe.builder.pipeline_builder import (
    BuiltPipeline,
    FlowChartPipelineSpec,
    TopologyValidator,
    apply_pipeline_queue_limits,
    build_flow_chart_pipeline_from_spec,
    build_flow_chart_pipeline_spec_from_graph,
    validate_flow_chart_pipeline_spec,
)
from simple_pipe.context.frame_meta import FrameInput, FrameMeta
from simple_pipe.enums import DropPolicy
from simple_pipe.operators.registry import NodeFactory, default_node_factory
from simple_pipe.result import Result
from simple_pipe.spec.graph_spec import PipelineGraphSpec
from simple_pipe.spec.runtime_config import PipelineRuntimeConfig


class PipelineState(str, Enum):
    UNBUILT = "Unbuilt"
    BUILT = "Built"
    CONFIGURED = "Configured"
    RUNNING = "Running"
    STOPPED = "Stopped"


class PipelineBase(ABC):
    """
    Pipeline runtime shell: session lifecycle Build → Configure → Start → PushFrame → Stop.
    """

    def __init__(self, node_factory: NodeFactory | None = None) -> None:
        self._state = PipelineState.UNBUILT
        self._graph_spec: PipelineGraphSpec | None = None
        self._flow_spec: FlowChartPipelineSpec | None = None
        self._built: BuiltPipeline | None = None
        self._runtime: PipelineRuntimeConfig | None = None
        self._output_callback: Callable[[FrameMeta], None] | None = None
        self._node_factory = node_factory or default_node_factory()

    @property
    def state(self) -> PipelineState:
        return self._state

    def version(self) -> str:
        from simple_pipe import __version__

        return __version__

    def is_running(self) -> bool:
        return self._state == PipelineState.RUNNING

    def set_output_callback(self, cb: Callable[[FrameMeta], None]) -> None:
        self._output_callback = cb
        if self._built:
            self._built.sink.set_output_callback(cb)

    def build(
        self,
        graph: PipelineGraphSpec,
        load_defaults: Callable[[str], dict] | None = None,
        domain_validator: TopologyValidator | None = None,
    ) -> Result[None]:
        flow = build_flow_chart_pipeline_spec_from_graph(graph, load_defaults)
        valid = validate_flow_chart_pipeline_spec(flow, domain_validator)
        if not valid.ok:
            return valid
        try:
            built = build_flow_chart_pipeline_from_spec(flow, self._node_factory)
        except Exception as exc:
            return Result.failure(str(exc))

        self._graph_spec = graph
        self._flow_spec = flow
        self._built = built
        if self._output_callback:
            built.sink.set_output_callback(self._output_callback)
        self._state = PipelineState.BUILT
        return Result.success()

    def configure(self, runtime: PipelineRuntimeConfig) -> Result[None]:
        if self._state == PipelineState.UNBUILT:
            return Result.failure("Pipeline not built")
        if self._built is None:
            return Result.failure("Internal error: missing built pipeline")

        apply_pipeline_queue_limits(self._built, runtime.pipeline_params, DropPolicy.DROP_NEWEST)

        for node_id, patch in runtime.node_params.items():
            op = self._built.node_map.get(node_id)
            if op is None:
                continue
            result = op.configure(patch)
            if not result.ok:
                return result

        self._runtime = runtime
        self._state = PipelineState.CONFIGURED
        return Result.success()

    def start(self) -> Result[None]:
        if self._state not in (PipelineState.BUILT, PipelineState.CONFIGURED, PipelineState.STOPPED):
            if self._state == PipelineState.RUNNING:
                return Result.success()
            return Result.failure(f"Cannot start from state {self._state}")
        if self._built is None:
            return Result.failure("Pipeline not built")

        for node in self._built.nodes:
            node.start()
        self._state = PipelineState.RUNNING
        return Result.success()

    def stop(self) -> None:
        if self._built:
            for node in self._built.nodes:
                node.stop()
        self._state = PipelineState.STOPPED

    def push_frame(self, frame: FrameInput) -> Result[None]:
        if self._state != PipelineState.RUNNING:
            return Result.failure("Pipeline is not running")
        if self._built is None:
            return Result.failure("Pipeline not built")

        meta = FrameMeta.from_input(frame)
        self._built.source.inject(meta)
        return Result.success()

    @abstractmethod
    def domain_name(self) -> str:
        raise NotImplementedError
