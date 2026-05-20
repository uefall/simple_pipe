#pragma once

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/media/media_sample.hpp"

namespace simple_pipe::media {

FrameMeta ToFrameMeta(const MediaSample& sample, const std::string& source_type, const std::string& node_id);

}  // namespace simple_pipe::media
