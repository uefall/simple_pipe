#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <opencv2/core.hpp>

namespace simple_pipe::media {

struct FrameBuffer {
  cv::Mat mat;
  std::string uri;
  int64_t pts_us = 0;
  bool eos = false;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;

}  // namespace simple_pipe::media
