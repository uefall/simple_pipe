#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "simple_pipe/builder/pipeline_builder.hpp"
#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/enums.hpp"
#include "simple_pipe/operators/node_registry.hpp"
#include "simple_pipe/result.hpp"
#include "simple_pipe/spec/graph_spec.hpp"
#include "simple_pipe/spec/runtime_config.hpp"

namespace simple_pipe {

class PipelineBase {
 public:
  explicit PipelineBase(NodeFactory factory = nullptr);
  virtual ~PipelineBase() = default;

  virtual std::string Version() const;
  virtual std::string DomainName() const = 0;

  PipelineState state() const { return state_; }
  bool IsRunning() const { return state_ == PipelineState::kRunning; }

  void SetOutputCallback(AppSink::OutputCallback cb);

  Result<void> Build(const PipelineGraphSpec& graph, DefaultsLoader load_defaults = nullptr,
                     TopologyValidator domain_validator = nullptr);

  Result<void> Configure(const PipelineRuntimeConfig& runtime);

  Result<void> StartPipeline();
  void StopPipeline();

  Result<void> PushFrame(const FrameInput& frame);

 protected:
  NodeFactory factory_;
  PipelineState state_ = PipelineState::kUnbuilt;
  std::optional<PipelineGraphSpec> graph_spec_;
  std::optional<FlowChartPipelineSpec> flow_spec_;
  std::optional<BuiltPipeline> built_;
  std::optional<PipelineRuntimeConfig> runtime_;
  AppSink::OutputCallback output_callback_;
};

}  // namespace simple_pipe
