#pragma once

#include <string>
#include <vector>

#include "simple_pipe/media/i_media_reader.hpp"

namespace simple_pipe::media {

class OpenCvImageReader : public IMediaReader {
 public:
  Result<void> Open(const nlohmann::json& params) override;
  std::optional<MediaSample> Next() override;
  void Close() override;

 private:
  std::optional<MediaSample> MakeEosSample();

  std::vector<std::string> paths_;
  size_t cursor_ = 0;
  int frame_index_ = 0;
  bool loop_ = false;
  int fps_ = 0;
};

}  // namespace simple_pipe::media
