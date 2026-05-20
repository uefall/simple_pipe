#include "simple_pipe/media/opencv_image_reader.hpp"

#include <algorithm>
#include <filesystem>

#include <opencv2/imgcodecs.hpp>

#include "simple_pipe/log/logger.hpp"

namespace simple_pipe::media {

namespace fs = std::filesystem;

namespace {

void CollectPaths(const std::string& entry, std::vector<std::string>& out) {
  if (entry.find('*') != std::string::npos) {
    const fs::path parent = fs::path(entry).parent_path();
    const std::string pattern = fs::path(entry).filename().string();
    if (!fs::exists(parent)) {
      return;
    }
    for (const auto& item : fs::directory_iterator(parent)) {
      if (!item.is_regular_file()) {
        continue;
      }
      const auto name = item.path().filename().string();
      if (pattern == "*" || pattern == "*.*" ||
          (pattern.size() > 1 && pattern[0] == '*' &&
           name.size() >= pattern.size() - 1 &&
           name.compare(name.size() - (pattern.size() - 1), pattern.size() - 1, pattern.substr(1)))) {
        out.push_back(item.path().string());
      }
    }
    std::sort(out.begin(), out.end());
    return;
  }

  if (fs::is_directory(entry)) {
    for (const auto& item : fs::directory_iterator(entry)) {
      if (item.is_regular_file()) {
        out.push_back(item.path().string());
      }
    }
    std::sort(out.begin(), out.end());
    return;
  }

  if (fs::is_regular_file(entry)) {
    out.push_back(entry);
  }
}

}  // namespace

Result<void> OpenCvImageReader::Open(const nlohmann::json& params) {
  paths_.clear();
  cursor_ = 0;
  frame_index_ = 0;

  if (params.contains("loop")) {
    loop_ = params["loop"].get<bool>();
  }
  if (params.contains("fps")) {
    fps_ = params["fps"].get<int>();
  }

  if (params.contains("paths") && params["paths"].is_array()) {
    for (const auto& item : params["paths"]) {
      CollectPaths(item.get<std::string>(), paths_);
    }
  } else if (params.contains("path")) {
    CollectPaths(params["path"].get<std::string>(), paths_);
  }

  if (paths_.empty()) {
    return Result<void>::failure("image_src: no input images resolved");
  }

  log::Info("OpenCvImageReader", "opened " + std::to_string(paths_.size()) + " image(s)");
  return Result<void>::success();
}

std::optional<MediaSample> OpenCvImageReader::MakeEosSample() {
  auto buffer = std::make_shared<FrameBuffer>();
  buffer->eos = true;
  buffer->uri = "eos";
  MediaSample sample;
  sample.buffer = buffer;
  sample.frame_index = frame_index_;
  return sample;
}

std::optional<MediaSample> OpenCvImageReader::Next() {
  if (cursor_ >= paths_.size()) {
    if (loop_) {
      cursor_ = 0;
    } else {
      return MakeEosSample();
    }
  }

  const std::string& path = paths_[cursor_++];
  cv::Mat mat = cv::imread(path, cv::IMREAD_COLOR);
  if (mat.empty()) {
    log::Warn("OpenCvImageReader", "failed to read image: " + path);
    return Next();
  }

  auto buffer = std::make_shared<FrameBuffer>();
  buffer->mat = mat;
  buffer->uri = path;
  buffer->pts_us = static_cast<int64_t>(frame_index_) * 1'000'000;

  MediaSample sample;
  sample.buffer = buffer;
  sample.frame_index = frame_index_++;
  return sample;
}

void OpenCvImageReader::Close() {
  paths_.clear();
  cursor_ = 0;
}

}  // namespace simple_pipe::media
