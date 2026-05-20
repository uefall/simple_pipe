#pragma once

#include <filesystem>
#include <string>

#ifndef SIMPLE_PIPE_SOURCE_DIR
#define SIMPLE_PIPE_SOURCE_DIR "."
#endif

namespace simple_pipe::test {

inline std::filesystem::path RepoPath(const std::string& relative) {
  return std::filesystem::path(SIMPLE_PIPE_SOURCE_DIR) / relative;
}

inline std::string GraphPath(const std::string& name) {
  return RepoPath("configs/demo/graph/" + name + ".json").string();
}

}  // namespace simple_pipe::test
