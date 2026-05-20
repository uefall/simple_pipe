#pragma once

#include <optional>

#include <nlohmann/json.hpp>

#include "simple_pipe/media/media_sample.hpp"
#include "simple_pipe/result.hpp"

namespace simple_pipe::media {

class IMediaReader {
 public:
  virtual ~IMediaReader() = default;

  virtual Result<void> Open(const nlohmann::json& params) = 0;
  virtual std::optional<MediaSample> Next() = 0;
  virtual void Close() = 0;
};

}  // namespace simple_pipe::media
