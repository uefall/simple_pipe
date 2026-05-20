#include "simple_pipe/builder/pipeline_builder.hpp"

#include <fstream>
#include <unordered_set>

namespace simple_pipe {

namespace {

bool IsIngressType(const std::string& type) {
  return type == "app_src" || type == "image_src" || type == "video_src";
}

nlohmann::json ShallowMerge(const nlohmann::json& base, const nlohmann::json& overlay) {
  nlohmann::json merged = base.is_object() ? base : nlohmann::json::object();
  if (overlay.is_object()) {
    for (auto it = overlay.begin(); it != overlay.end(); ++it) {
      merged[it.key()] = it.value();
    }
  }
  return merged;
}

}  // namespace

const FlowChartNodeSpec* FlowChartPipelineSpec::Find(const std::string& id) const {
  for (const auto& node : nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

nlohmann::json LoadNodeDefaults(const std::string& defaults_dir, const std::string& node_type) {
  const std::string path = defaults_dir + "/" + node_type + ".json";
  std::ifstream in(path);
  if (!in) {
    return nlohmann::json::object();
  }
  nlohmann::json data;
  in >> data;
  return data;
}

FlowChartPipelineSpec BuildFlowChartPipelineSpecFromGraph(const PipelineGraphSpec& graph,
                                                           DefaultsLoader load_defaults) {
  std::unordered_map<std::string, std::vector<std::string>> upstream_map;
  for (const auto& node : graph.nodes) {
    upstream_map[node.id] = {};
  }
  for (const auto& edge : graph.edges) {
    upstream_map[edge.to].insert(upstream_map[edge.to].end(), edge.from_nodes.begin(),
                                 edge.from_nodes.end());
  }

  FlowChartPipelineSpec spec;
  for (const auto& node : graph.nodes) {
    nlohmann::json defaults = nlohmann::json::object();
    if (load_defaults) {
      defaults = load_defaults(node.type);
    }
    FlowChartNodeSpec flow;
    flow.id = node.id;
    flow.type = node.type;
    flow.init_params = ShallowMerge(defaults, node.init_params);
    flow.upstream_ids = upstream_map[node.id];
    spec.nodes.push_back(std::move(flow));
  }
  return spec;
}

Result<void> ValidateFlowChartPipelineSpec(const FlowChartPipelineSpec& spec,
                                           TopologyValidator domain_rules) {
  std::unordered_set<std::string> ids;
  for (const auto& node : spec.nodes) {
    if (!ids.insert(node.id).second) {
      return Result<void>::failure("Duplicate node id in graph");
    }
  }

  for (const auto& node : spec.nodes) {
    for (const auto& upstream : node.upstream_ids) {
      if (ids.find(upstream) == ids.end()) {
        return Result<void>::failure("Edge references unknown upstream: " + upstream);
      }
    }
    if (IsIngressType(node.type) && !node.upstream_ids.empty()) {
      return Result<void>::failure("ingress source must not have upstream edges: " + node.type);
    }
  }

  int source_count = 0;
  int sink_count = 0;
  for (const auto& node : spec.nodes) {
    if (IsIngressType(node.type)) {
      ++source_count;
    }
    if (node.type == "app_des") {
      ++sink_count;
    }
  }
  if (source_count != 1) {
    return Result<void>::failure(
        "Graph must contain exactly one ingress source (app_src | image_src | video_src)");
  }
  if (sink_count != 1) {
    return Result<void>::failure("Graph must contain exactly one app_des");
  }

  if (domain_rules) {
    return domain_rules(spec);
  }
  return Result<void>::success();
}

BuiltPipeline BuildFlowChartPipelineFromSpec(const FlowChartPipelineSpec& spec, NodeFactory factory) {
  if (!factory) {
    factory = DefaultNodeFactory();
  }

  BuiltPipeline built;
  for (const auto& node_spec : spec.nodes) {
    auto op = factory(node_spec.id, node_spec.type, node_spec.init_params);
    built.node_map[node_spec.id] = op;
    built.nodes.push_back(op);
  }

  for (const auto& node_spec : spec.nodes) {
    auto op = built.node_map.at(node_spec.id);
    std::vector<std::shared_ptr<Operator>> upstreams;
    for (const auto& uid : node_spec.upstream_ids) {
      upstreams.push_back(built.node_map.at(uid));
    }
    op->AttachTo(upstreams);
    for (const auto& up : upstreams) {
      up->AddDownstream(op);
    }
  }

  for (const auto& node : built.nodes) {
    if (node->role() == NodeRole::kSrc) {
      built.source = std::dynamic_pointer_cast<SourceOperator>(node);
      if (built.source) {
        built.pull_driven = built.source->PullDriven();
      }
    }
    if (node->role() == NodeRole::kDes) {
      built.sink = std::dynamic_pointer_cast<AppSink>(node);
    }
  }

  if (!built.source || !built.sink) {
    throw std::runtime_error("Built graph must have exactly one ingress source and one AppSink");
  }
  return built;
}

void ApplyPipelineQueueLimits(BuiltPipeline& built, const nlohmann::json& pipeline_params,
                              DropPolicy policy) {
  int src_limit = 8;
  int des_limit = 8;
  if (pipeline_params.contains("app_src_in_queue_limit")) {
    src_limit = pipeline_params["app_src_in_queue_limit"].get<int>();
  } else if (pipeline_params.contains("source_queue_limit")) {
    src_limit = pipeline_params["source_queue_limit"].get<int>();
  }
  if (pipeline_params.contains("app_des_in_queue_limit")) {
    des_limit = pipeline_params["app_des_in_queue_limit"].get<int>();
  } else if (pipeline_params.contains("sink_queue_limit")) {
    des_limit = pipeline_params["sink_queue_limit"].get<int>();
  }
  built.source->SetInputQueueLimit(src_limit, policy);
  built.sink->SetInputQueueLimit(des_limit, policy);
}

}  // namespace simple_pipe
