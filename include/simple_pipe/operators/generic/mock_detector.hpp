#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class MockDetector : public Operator {
 public:
  MockDetector(std::string node_id, nlohmann::json build_init_params);
  NodeRole role() const override { return NodeRole::kMid; }
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;

 private:
  std::string label_ = "object";
};

}  // namespace simple_pipe
