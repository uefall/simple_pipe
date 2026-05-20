#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class MockSafetyRules : public Operator {
 public:
  MockSafetyRules(std::string node_id, nlohmann::json build_init_params);

  NodeRole role() const override { return NodeRole::kMid; }
  Result<void> Configure(const nlohmann::json& patch) override;
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;

 private:
  double min_score_ = 0.5;
};

}  // namespace simple_pipe
