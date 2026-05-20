#include "simple_pipe/operators/generic/video_source.hpp"

#include "simple_pipe/media/opencv_video_reader.hpp"

namespace simple_pipe {

VideoSource::VideoSource(std::string node_id, nlohmann::json build_init_params)
    : PullSource(std::move(node_id), "video_src", std::move(build_init_params)) {}

std::unique_ptr<media::IMediaReader> VideoSource::CreateReader() {
  return std::make_unique<media::OpenCvVideoReader>();
}

}  // namespace simple_pipe
