#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace simple_pipe {

struct PipelineRuntimeConfig {
  nlohmann::json pipeline_params = nlohmann::json::object();
  std::unordered_map<std::string, nlohmann::json> node_params;

  static PipelineRuntimeConfig FromJson(const nlohmann::json& data);
  static PipelineRuntimeConfig LoadFromFile(const std::string& path);
};

}  // namespace simple_pipe
