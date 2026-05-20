#include "simple_pipe/operators/generic/mock_tracker.hpp"

namespace simple_pipe {

MockTracker::MockTracker(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "mock_tracker", std::move(build_init_params)) {}

FrameMeta MockTracker::HandleFrameMeta(const FrameMeta& meta) {
  FrameMeta out = meta.ShallowCopy();
  nlohmann::json track = {{"id", 1}, {"detection_count", out.detections.size()}};
  if (out.extensions.contains("motion")) {
    track["motion"] = out.extensions["motion"];
  }
  out.extensions["tracks"] = nlohmann::json::array({track});
  return out;
}

}  // namespace simple_pipe
