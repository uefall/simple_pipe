#pragma once

#include <memory>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "simple_pipe/media/frame_buffer.hpp"

namespace simple_pipe {

struct FrameInput {
  std::string image;
  int frame_index = 0;
  int channel_index = 0;
  int program_code = 0;
  nlohmann::json extensions = nlohmann::json::object();
};

struct FrameMeta {
  nlohmann::json images = nlohmann::json::object();
  int frame_index = 0;
  int channel_index = 0;
  int program_code = 0;
  std::vector<nlohmann::json> detections;
  nlohmann::json extensions = nlohmann::json::object();
  media::FrameBufferPtr buffer;

  static FrameMeta FromInput(const FrameInput& input);
  FrameMeta ShallowCopy() const;
  bool IsEos() const;
};

}  // namespace simple_pipe
