#pragma once

#include <opencv2/videoio.hpp>

#include "simple_pipe/media/i_media_reader.hpp"

namespace simple_pipe::media {

class OpenCvVideoReader : public IMediaReader {
 public:
  Result<void> Open(const nlohmann::json& params) override;
  std::optional<MediaSample> Next() override;
  void Close() override;

 private:
  cv::VideoCapture capture_;
  int frame_index_ = 0;
  double target_fps_ = 0.0;
  int max_frames_ = -1;
  int64_t frame_duration_us_ = 0;
};

}  // namespace simple_pipe::media
