#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class AppSource : public Operator {
 public:
  AppSource(std::string node_id, nlohmann::json build_init_params);

  NodeRole role() const override { return NodeRole::kSrc; }
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;
  void Inject(FrameMeta meta);
};

}  // namespace simple_pipe
