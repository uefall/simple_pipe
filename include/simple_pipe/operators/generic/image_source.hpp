#pragma once

#include "simple_pipe/operators/generic/pull_source.hpp"

namespace simple_pipe {

class ImageSource : public PullSource {
 public:
  ImageSource(std::string node_id, nlohmann::json build_init_params);

 protected:
  std::unique_ptr<media::IMediaReader> CreateReader() override;
};

}  // namespace simple_pipe
