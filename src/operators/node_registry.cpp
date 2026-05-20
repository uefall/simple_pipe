#include "simple_pipe/operators/node_registry.hpp"

#include <unordered_map>

#include "simple_pipe/operators/generic/app_sink.hpp"
#include "simple_pipe/operators/generic/app_source.hpp"
#include "simple_pipe/operators/generic/mock_detector.hpp"
#include "simple_pipe/operators/generic/mock_motion.hpp"
#include "simple_pipe/operators/generic/mock_safety_rules.hpp"
#include "simple_pipe/operators/generic/mock_tracker.hpp"
#include "simple_pipe/operators/generic/passthrough.hpp"
#include "simple_pipe/operators/generic/resize.hpp"

namespace simple_pipe {

NodeFactory DefaultNodeFactory() {
  auto registry = std::make_shared<std::unordered_map<std::string, NodeFactory>>();

  auto reg = [&](const std::string& type, NodeFactory factory) { (*registry)[type] = std::move(factory); };

  reg("app_src", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<AppSource>(id, p);
  });
  reg("app_des", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<AppSink>(id, p);
  });
  reg("passthrough", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<Passthrough>(id, p);
  });
  reg("resize", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<Resize>(id, p);
  });
  reg("mock_detector", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<MockDetector>(id, p);
  });
  reg("mock_motion", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<MockMotion>(id, p);
  });
  reg("mock_tracker", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<MockTracker>(id, p);
  });
  reg("mock_safety_rules", [](const std::string& id, const std::string&, const nlohmann::json& p) {
    return std::make_shared<MockSafetyRules>(id, p);
  });

  return [registry](const std::string& node_id, const std::string& node_type,
                    const nlohmann::json& init_params) -> std::shared_ptr<Operator> {
    auto it = registry->find(node_type);
    if (it == registry->end()) {
      throw std::runtime_error("Unknown node type: " + node_type);
    }
    return it->second(node_id, node_type, init_params);
  };
}

}  // namespace simple_pipe
