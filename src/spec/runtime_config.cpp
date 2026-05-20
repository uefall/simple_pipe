#include "simple_pipe/spec/runtime_config.hpp"

#include <fstream>
#include <stdexcept>

namespace simple_pipe {

PipelineRuntimeConfig PipelineRuntimeConfig::FromJson(const nlohmann::json& data) {
  PipelineRuntimeConfig cfg;
  if (data.contains("pipeline_params")) {
    cfg.pipeline_params = data.at("pipeline_params");
  }
  const nlohmann::json* nodes = nullptr;
  if (data.contains("node_params")) {
    nodes = &data.at("node_params");
  } else if (data.contains("nodes")) {
    nodes = &data.at("nodes");
  }
  if (nodes != nullptr && nodes->is_object()) {
    for (auto it = nodes->begin(); it != nodes->end(); ++it) {
      cfg.node_params[it.key()] = it.value();
    }
  }
  return cfg;
}

PipelineRuntimeConfig PipelineRuntimeConfig::LoadFromFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Failed to open runtime config: " + path);
  }
  nlohmann::json data;
  in >> data;
  return FromJson(data);
}

}  // namespace simple_pipe
