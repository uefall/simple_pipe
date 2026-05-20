#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>
#include <vector>

namespace simple_pipe {

struct GraphNodeSpec {
  std::string id;
  std::string type;
  nlohmann::json init_params = nlohmann::json::object();
};

struct GraphEdgeSpec {
  std::string to;
  std::vector<std::string> from_nodes;
};

struct PipelineGraphSpec {
  std::vector<GraphNodeSpec> nodes;
  std::vector<GraphEdgeSpec> edges;

  std::unordered_set<std::string> NodeIds() const;
  const GraphNodeSpec* FindNode(const std::string& id) const;
};

PipelineGraphSpec PipelineGraphSpecFromJson(const nlohmann::json& data);
PipelineGraphSpec LoadPipelineGraphSpecFromJson(const std::string& path);

}  // namespace simple_pipe
