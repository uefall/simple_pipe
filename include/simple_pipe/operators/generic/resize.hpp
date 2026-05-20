#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class Resize : public Operator {
 public:
  Resize(std::string node_id, nlohmann::json build_init_params);

  NodeRole role() const override { return NodeRole::kMid; }
  Result<void> Configure(const nlohmann::json& patch) override;
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;

 private:
  int width_ = 640;
  int height_ = 480;
};

}  // namespace simple_pipe
