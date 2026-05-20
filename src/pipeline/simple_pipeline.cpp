#include "simple_pipe/pipeline/simple_pipeline.hpp"

namespace simple_pipe {

SimplePipeline::SimplePipeline(std::string defaults_dir)
    : PipelineBase(), defaults_dir_(std::move(defaults_dir)) {}

Result<void> SimplePipeline::BuildFromJson(const std::string& graph_path) {
  try {
    auto graph = LoadPipelineGraphSpecFromJson(graph_path);
    DefaultsLoader loader = nullptr;
    if (!defaults_dir_.empty()) {
      loader = [dir = defaults_dir_](const std::string& type) {
        return LoadNodeDefaults(dir, type);
      };
    }
    TopologyValidator validator = topology_validator_;
    if (!validator) {
      validator = [](const FlowChartPipelineSpec&) { return Result<void>::success(); };
    }
    return Build(graph, std::move(loader), std::move(validator));
  } catch (const std::exception& ex) {
    return Result<void>::failure(ex.what());
  }
}

void SimplePipeline::SetTopologyValidator(TopologyValidator validator) {
  topology_validator_ = std::move(validator);
}

}  // namespace simple_pipe
