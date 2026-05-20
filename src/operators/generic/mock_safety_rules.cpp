#include "simple_pipe/operators/generic/mock_safety_rules.hpp"

namespace simple_pipe {

MockSafetyRules::MockSafetyRules(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "mock_safety_rules", std::move(build_init_params)) {
  if (build_init_params_.contains("min_score")) {
    min_score_ = build_init_params_["min_score"].get<double>();
  }
}

Result<void> MockSafetyRules::Configure(const nlohmann::json& patch) {
  if (patch.contains("min_score")) {
    min_score_ = patch["min_score"].get<double>();
  }
  return Result<void>::success();
}

FrameMeta MockSafetyRules::HandleFrameMeta(const FrameMeta& meta) {
  FrameMeta out = meta.ShallowCopy();
  nlohmann::json alerts = nlohmann::json::array();
  for (const auto& det : out.detections) {
    if (det.contains("score") && det["score"].get<double>() >= min_score_) {
      alerts.push_back(det);
    }
  }
  out.extensions["safety"] = {{"alert", !alerts.empty()}, {"alerts", alerts}};
  return out;
}

}  // namespace simple_pipe
