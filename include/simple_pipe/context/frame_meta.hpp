#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

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

  static FrameMeta FromInput(const FrameInput& input);
  FrameMeta ShallowCopy() const;
};

}  // namespace simple_pipe
