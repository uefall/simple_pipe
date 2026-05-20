#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class MockTracker : public Operator {
 public:
  MockTracker(std::string node_id, nlohmann::json build_init_params);
  NodeRole role() const override { return NodeRole::kMid; }
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;
};

}  // namespace simple_pipe
