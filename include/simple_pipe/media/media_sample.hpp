#pragma once

#include "simple_pipe/media/frame_buffer.hpp"

namespace simple_pipe::media {

struct MediaSample {
  FrameBufferPtr buffer;
  int frame_index = 0;
  int channel_index = 0;
};

}  // namespace simple_pipe::media
