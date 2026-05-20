#include "simple_pipe/operators/generic/app_source.hpp"

namespace simple_pipe {

AppSource::AppSource(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "app_src", std::move(build_init_params)) {}

FrameMeta AppSource::HandleFrameMeta(const FrameMeta& meta) { return meta; }

void AppSource::Inject(FrameMeta meta) {
  if (!RunningFlag().load()) {
    return;
  }
  PushOutput(HandleFrameMeta(meta));
}

}  // namespace simple_pipe
