#pragma once

#include <string>

#include "simple_pipe/pipeline/pipeline_base.hpp"

namespace simple_pipe {

class SimplePipeline : public PipelineBase {
 public:
  explicit SimplePipeline(std::string defaults_dir = "");

  std::string DomainName() const override { return "simple"; }

  Result<void> BuildFromJson(const std::string& graph_path);
  void SetTopologyValidator(TopologyValidator validator);

 private:
  std::string defaults_dir_;
  TopologyValidator topology_validator_;
};

}  // namespace simple_pipe
