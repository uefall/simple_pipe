#include "simple_pipe/pipeline/pipeline_base.hpp"

namespace simple_pipe {

PipelineBase::PipelineBase(NodeFactory factory)
    : factory_(factory ? std::move(factory) : DefaultNodeFactory()) {}

std::string PipelineBase::Version() const { return "0.2.0"; }

void PipelineBase::SetOutputCallback(AppSink::OutputCallback cb) {
  output_callback_ = std::move(cb);
  if (built_) {
    built_->sink->SetOutputCallback(output_callback_);
  }
}

Result<void> PipelineBase::Build(const PipelineGraphSpec& graph, DefaultsLoader load_defaults,
                                 TopologyValidator domain_validator) {
  auto flow = BuildFlowChartPipelineSpecFromGraph(graph, std::move(load_defaults));
  auto valid = ValidateFlowChartPipelineSpec(flow, std::move(domain_validator));
  if (!valid.IsSuccess()) {
    return valid;
  }

  try {
    built_ = BuildFlowChartPipelineFromSpec(flow, factory_);
  } catch (const std::exception& ex) {
    return Result<void>::failure(ex.what());
  }

  graph_spec_ = graph;
  flow_spec_ = std::move(flow);
  if (output_callback_) {
    built_->sink->SetOutputCallback(output_callback_);
  }
  state_ = PipelineState::kBuilt;
  return Result<void>::success();
}

Result<void> PipelineBase::Configure(const PipelineRuntimeConfig& runtime) {
  if (state_ == PipelineState::kUnbuilt || !built_) {
    return Result<void>::failure("Pipeline not built");
  }

  ApplyPipelineQueueLimits(*built_, runtime.pipeline_params);
  for (const auto& [node_id, patch] : runtime.node_params) {
    auto it = built_->node_map.find(node_id);
    if (it == built_->node_map.end()) {
      continue;
    }
    auto result = it->second->Configure(patch);
    if (!result.IsSuccess()) {
      return result;
    }
  }

  runtime_ = runtime;
  if (state_ != PipelineState::kRunning) {
    state_ = PipelineState::kConfigured;
  }
  return Result<void>::success();
}

Result<void> PipelineBase::StartPipeline() {
  if (state_ == PipelineState::kRunning) {
    return Result<void>::success();
  }
  if (state_ != PipelineState::kBuilt && state_ != PipelineState::kConfigured &&
      state_ != PipelineState::kStopped) {
    return Result<void>::failure("Cannot start pipeline from current state");
  }
  if (!built_) {
    return Result<void>::failure("Pipeline not built");
  }

  for (auto& node : built_->nodes) {
    node->Start();
  }
  state_ = PipelineState::kRunning;
  return Result<void>::success();
}

void PipelineBase::StopPipeline() {
  if (built_) {
    for (auto& node : built_->nodes) {
      node->Stop();
    }
  }
  state_ = PipelineState::kStopped;
}

Result<void> PipelineBase::PushFrame(const FrameInput& frame) {
  if (state_ != PipelineState::kRunning) {
    return Result<void>::failure("Pipeline is not running");
  }
  if (!built_) {
    return Result<void>::failure("Pipeline not built");
  }
  if (built_->pull_driven) {
    return Result<void>::failure(
        "PushFrame is not allowed when ingress is pull-driven (image_src/video_src)");
  }
  built_->source->Inject(FrameMeta::FromInput(frame));
  return Result<void>::success();
}

}  // namespace simple_pipe
