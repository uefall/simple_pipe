#include "simple_pipe/media/opencv_video_reader.hpp"

#include <chrono>
#include <thread>

#include "simple_pipe/log/logger.hpp"

namespace simple_pipe::media {

Result<void> OpenCvVideoReader::Open(const nlohmann::json& params) {
  frame_index_ = 0;

  std::string uri;
  if (params.contains("uri")) {
    uri = params["uri"].get<std::string>();
  } else if (params.contains("path")) {
    uri = params["path"].get<std::string>();
  }

  if (uri.empty() && params.contains("device")) {
    capture_.open(params["device"].get<int>());
  } else if (!uri.empty()) {
    capture_.open(uri);
  } else {
    return Result<void>::failure("video_src: missing uri/path/device");
  }

  if (!capture_.isOpened()) {
    return Result<void>::failure("video_src: failed to open capture");
  }

  if (params.contains("target_fps")) {
    target_fps_ = params["target_fps"].get<double>();
  } else {
    target_fps_ = capture_.get(cv::CAP_PROP_FPS);
    if (target_fps_ <= 1.0) {
      target_fps_ = 25.0;
    }
  }
  frame_duration_us_ = target_fps_ > 0 ? static_cast<int64_t>(1'000'000.0 / target_fps_) : 0;

  if (params.contains("max_frames")) {
    max_frames_ = params["max_frames"].get<int>();
  }

  log::Info("OpenCvVideoReader", "opened uri=" + uri + " target_fps=" + std::to_string(target_fps_));
  return Result<void>::success();
}

std::optional<MediaSample> OpenCvVideoReader::Next() {
  if (max_frames_ >= 0 && frame_index_ >= max_frames_) {
    auto buffer = std::make_shared<FrameBuffer>();
    buffer->eos = true;
    buffer->uri = "eos";
    MediaSample sample;
    sample.buffer = buffer;
    sample.frame_index = frame_index_;
    return sample;
  }

  cv::Mat mat;
  if (!capture_.read(mat) || mat.empty()) {
    auto buffer = std::make_shared<FrameBuffer>();
    buffer->eos = true;
    buffer->uri = "eos";
    MediaSample sample;
    sample.buffer = buffer;
    sample.frame_index = frame_index_;
    return sample;
  }

  if (frame_duration_us_ > 0 && frame_index_ > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(frame_duration_us_));
  }

  auto buffer = std::make_shared<FrameBuffer>();
  buffer->mat = mat;
  buffer->uri = "frame";
  buffer->pts_us = static_cast<int64_t>(frame_index_) * frame_duration_us_;

  MediaSample sample;
  sample.buffer = buffer;
  sample.frame_index = frame_index_++;
  return sample;
}

void OpenCvVideoReader::Close() {
  if (capture_.isOpened()) {
    capture_.release();
  }
}

}  // namespace simple_pipe::media
