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
  int out_w = width_;
  int out_h = height_;
  if (out.buffer && !out.buffer->mat.empty()) {
    out_w = width_ > 0 ? width_ : out.buffer->mat.cols;
    out_h = height_ > 0 ? height_ : out.buffer->mat.rows;
  }
  out.extensions["resize"] = {{"width", out_w}, {"height", out_h}};
  return out;
}

}  // namespace simple_pipe
