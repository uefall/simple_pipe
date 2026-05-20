#include "simple_pipe/operators/generic/mock_motion.hpp"

namespace simple_pipe {

MockMotion::MockMotion(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "mock_motion", std::move(build_init_params)) {}

FrameMeta MockMotion::HandleFrameMeta(const FrameMeta& meta) {
  FrameMeta out = meta.ShallowCopy();
  out.extensions["motion"] = {{"moving", meta.frame_index % 2 == 0}};
  return out;
}

}  // namespace simple_pipe
