#pragma once

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class SourceOperator : public Operator {
 public:
  using Operator::Operator;

  NodeRole role() const override { return NodeRole::kSrc; }
  virtual void Inject(FrameMeta meta) = 0;
  virtual bool PullDriven() const = 0;
};

}  // namespace simple_pipe
