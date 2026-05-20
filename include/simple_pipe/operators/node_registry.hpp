#pragma once

#include <functional>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "simple_pipe/operators/operator.hpp"

namespace simple_pipe {

using NodeFactory = std::function<std::shared_ptr<Operator>(
    const std::string& node_id, const std::string& node_type, const nlohmann::json& init_params)>;

NodeFactory DefaultNodeFactory();

}  // namespace simple_pipe
