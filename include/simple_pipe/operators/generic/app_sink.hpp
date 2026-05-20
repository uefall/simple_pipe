#pragma once

#include <functional>

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

class AppSink : public Operator {
 public:
  using OutputCallback = std::function<void(const FrameMeta&)>;

  AppSink(std::string node_id, nlohmann::json build_init_params);

  NodeRole role() const override { return NodeRole::kDes; }
  bool emits_downstream() const override { return false; }

  void SetOutputCallback(OutputCallback cb);
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;

 private:
  OutputCallback callback_;
};

}  // namespace simple_pipe
