#include "simple_pipe/media/frame_meta_builder.hpp"

namespace simple_pipe::media {

FrameMeta ToFrameMeta(const MediaSample& sample, const std::string& source_type,
                      const std::string& node_id) {
  FrameMeta meta;
  meta.frame_index = sample.frame_index;
  meta.channel_index = sample.channel_index;
  meta.buffer = sample.buffer;

  if (sample.buffer) {
    meta.images["main"] = sample.buffer->uri;
    meta.extensions["source"] = {
        {"type", source_type},
        {"node_id", node_id},
        {"uri", sample.buffer->uri},
    };
    meta.extensions["timing"] = {
        {"pts_us", sample.buffer->pts_us},
        {"eos", sample.buffer->eos},
    };
  }
  return meta;
}

}  // namespace simple_pipe::media
