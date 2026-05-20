#include "simple_pipe/operators/generic/mock_detector.hpp"

namespace simple_pipe {

MockDetector::MockDetector(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "mock_detector", std::move(build_init_params)) {
  if (build_init_params_.contains("label")) {
    label_ = build_init_params_["label"].get<std::string>();
  }
}

FrameMeta MockDetector::HandleFrameMeta(const FrameMeta& meta) {
  FrameMeta out = meta.ShallowCopy();
  out.detections.push_back(
      {{"source", node_id()}, {"label", label_}, {"score", 0.9}});
  return out;
}

}  // namespace simple_pipe
