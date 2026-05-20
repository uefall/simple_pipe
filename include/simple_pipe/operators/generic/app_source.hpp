#pragma once

#include "simple_pipe/operators/generic/source_operator.hpp"

namespace simple_pipe {

class AppSource : public SourceOperator {
 public:
  AppSource(std::string node_id, nlohmann::json build_init_params);

  bool PullDriven() const override { return false; }
  void Inject(FrameMeta meta) override;
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;
};

}  // namespace simple_pipe
