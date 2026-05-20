#include "simple_pipe/operators/generic/app_sink.hpp"

namespace simple_pipe {

AppSink::AppSink(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "app_des", std::move(build_init_params)) {}

void AppSink::SetOutputCallback(OutputCallback cb) { callback_ = std::move(cb); }

FrameMeta AppSink::HandleFrameMeta(const FrameMeta& meta) {
  if (callback_) {
    callback_(meta);
  }
  return meta;
}

}  // namespace simple_pipe
