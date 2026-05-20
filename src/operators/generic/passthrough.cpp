#include "simple_pipe/operators/generic/passthrough.hpp"

namespace simple_pipe {

Passthrough::Passthrough(std::string node_id, nlohmann::json build_init_params)
    : Operator(std::move(node_id), "passthrough", std::move(build_init_params)) {}

FrameMeta Passthrough::HandleFrameMeta(const FrameMeta& meta) { return meta.ShallowCopy(); }

}  // namespace simple_pipe
