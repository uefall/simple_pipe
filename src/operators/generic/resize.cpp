#include "simple_pipe/operators/generic/resize.hpp"

namespace simple_pipe {

Resize::Resize(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "resize", std::move(build_init_params)) {
  if (build_init_params_.contains("width")) {
    width_ = build_init_params_["width"].get<int>();
  }
  if (build_init_params_.contains("height")) {
    height_ = build_init_params_["height"].get<int>();
  }
}

Result<void> Resize::Configure(const nlohmann::json& patch) {
  if (patch.contains("width")) {
    width_ = patch["width"].get<int>();
  }
  if (patch.contains("height")) {
    height_ = patch["height"].get<int>();
  }
  return Result<void>::success();
}

FrameMeta Resize::HandleFrameMeta(const FrameMeta& meta) {
  FrameMeta out = meta.ShallowCopy();
  out.extensions["resize"] = {{"width", width_}, {"height", height_}};
  return out;
}

}  // namespace simple_pipe
