#include "simple_pipe/spec/graph_spec.hpp"

#include <fstream>
#include <stdexcept>

namespace simple_pipe {

namespace {

GraphNodeSpec ParseNode(const nlohmann::json& raw) {
  GraphNodeSpec node;
  node.id = raw.at("id").get<std::string>();
  node.type = raw.at("type").get<std::string>();
  if (raw.contains("init_params")) {
    node.init_params = raw.at("init_params");
  } else if (raw.contains("initParams")) {
    node.init_params = raw.at("initParams");
  }
  return node;
}

GraphEdgeSpec ParseEdge(const nlohmann::json& raw) {
  GraphEdgeSpec edge;
  edge.to = raw.at("to").get<std::string>();
  if (raw.contains("from")) {
    const auto& from = raw.at("from");
    if (from.is_string()) {
      edge.from_nodes.push_back(from.get<std::string>());
    } else {
      for (const auto& item : from) {
        edge.from_nodes.push_back(item.get<std::string>());
      }
    }
  } else if (raw.contains("from_nodes")) {
    for (const auto& item : raw.at("from_nodes")) {
      edge.from_nodes.push_back(item.get<std::string>());
    }
  }
  return edge;
}

}  // namespace

std::unordered_set<std::string> PipelineGraphSpec::NodeIds() const {
  std::unordered_set<std::string> ids;
  for (const auto& node : nodes) {
    ids.insert(node.id);
  }
  return ids;
}

const GraphNodeSpec* PipelineGraphSpec::FindNode(const std::string& id) const {
  for (const auto& node : nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

PipelineGraphSpec PipelineGraphSpecFromJson(const nlohmann::json& data) {
  PipelineGraphSpec spec;
  for (const auto& raw : data.at("nodes")) {
    spec.nodes.push_back(ParseNode(raw));
  }
  for (const auto& raw : data.at("edges")) {
    spec.edges.push_back(ParseEdge(raw));
  }
  return spec;
}

PipelineGraphSpec LoadPipelineGraphSpecFromJson(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Failed to open graph spec: " + path);
  }
  nlohmann::json data;
  in >> data;
  return PipelineGraphSpecFromJson(data);
}

}  // namespace simple_pipe
