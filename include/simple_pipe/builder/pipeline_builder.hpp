#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "simple_pipe/operators/generic/app_sink.hpp"
#include "simple_pipe/operators/generic/source_operator.hpp"
#include "simple_pipe/operators/node_registry.hpp"
#include "simple_pipe/result.hpp"
#include "simple_pipe/spec/graph_spec.hpp"
#include "simple_pipe/spec/runtime_config.hpp"

namespace simple_pipe {

struct FlowChartNodeSpec {
  std::string id;
  std::string type;
  nlohmann::json init_params;
  std::vector<std::string> upstream_ids;
};

struct FlowChartPipelineSpec {
  std::vector<FlowChartNodeSpec> nodes;
  const FlowChartNodeSpec* Find(const std::string& id) const;
};

struct BuiltPipeline {
  std::shared_ptr<SourceOperator> source;
  bool pull_driven = false;
  std::shared_ptr<AppSink> sink;
  std::vector<std::shared_ptr<Operator>> nodes;
  std::unordered_map<std::string, std::shared_ptr<Operator>> node_map;
};

using TopologyValidator = std::function<Result<void>(const FlowChartPipelineSpec&)>;

using DefaultsLoader = std::function<nlohmann::json(const std::string& node_type)>;

nlohmann::json LoadNodeDefaults(const std::string& defaults_dir, const std::string& node_type);

FlowChartPipelineSpec BuildFlowChartPipelineSpecFromGraph(
    const PipelineGraphSpec& graph, DefaultsLoader load_defaults = nullptr);

Result<void> ValidateFlowChartPipelineSpec(const FlowChartPipelineSpec& spec,
                                           TopologyValidator domain_rules = nullptr);

BuiltPipeline BuildFlowChartPipelineFromSpec(const FlowChartPipelineSpec& spec,
                                             NodeFactory factory = nullptr);

void ApplyPipelineQueueLimits(BuiltPipeline& built, const nlohmann::json& pipeline_params,
                              DropPolicy policy = DropPolicy::kDropNewest);

}  // namespace simple_pipe
