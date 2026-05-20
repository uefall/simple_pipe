#include "simple_pipe/operators/generic/image_source.hpp"

#include "simple_pipe/media/opencv_image_reader.hpp"

namespace simple_pipe {

ImageSource::ImageSource(std::string node_id, nlohmann::json build_init_params)
    : PullSource(std::move(node_id), "image_src", std::move(build_init_params)) {}

std::unique_ptr<media::IMediaReader> ImageSource::CreateReader() {
  return std::make_unique<media::OpenCvImageReader>();
}

}  // namespace simple_pipe
