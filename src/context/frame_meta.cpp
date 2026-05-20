#include "simple_pipe/context/frame_meta.hpp"

namespace simple_pipe {

FrameMeta FrameMeta::FromInput(const FrameInput& input) {
  FrameMeta meta;
  if (!input.image.empty()) {
    meta.images["main"] = input.image;
  }
  meta.frame_index = input.frame_index;
  meta.channel_index = input.channel_index;
  meta.program_code = input.program_code;
  meta.extensions = input.extensions;
  return meta;
}

FrameMeta FrameMeta::ShallowCopy() const {
  FrameMeta copy;
  copy.images = images;
  copy.frame_index = frame_index;
  copy.channel_index = channel_index;
  copy.program_code = program_code;
  copy.detections = detections;
  copy.extensions = extensions;
  return copy;
}

}  // namespace simple_pipe
