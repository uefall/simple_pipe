#include <chrono>
#include <iostream>
#include <thread>

#include <nlohmann/json.hpp>

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/pipeline/simple_pipeline.hpp"

#ifndef SIMPLE_PIPE_SOURCE_DIR
#define SIMPLE_PIPE_SOURCE_DIR "."
#endif

int main() {
  const std::string root = SIMPLE_PIPE_SOURCE_DIR;
  simple_pipe::SimplePipeline linear(root + "/configs/demo/node_defaults");
  if (!(linear.BuildFromJson(root + "/configs/demo/graph/linear.json")).IsSuccess()) {
    std::cerr << "linear build failed\n";
    return 1;
  }

  simple_pipe::SimplePipeline safety;
  if (!(safety.BuildFromJson(root + "/configs/demo/graph/safety_fanin.json")).IsSuccess()) {
    std::cerr << "safety build failed\n";
    return 1;
  }

  int count = 0;
  linear.SetOutputCallback([&](const simple_pipe::FrameMeta& meta) {
    ++count;
    if (count <= 2) {
      std::cout << "[linear] frame=" << meta.frame_index
                << " resize=" << meta.extensions.value("resize", nlohmann::json::object())
                << '\n';
    }
  });
  linear.StartPipeline();
  for (int i = 0; i < 2; ++i) {
    simple_pipe::FrameInput input;
    input.image = "demo";
    input.frame_index = i;
    linear.PushFrame(input);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  linear.StopPipeline();

  std::cout << "Demo OK — graphs: linear, safety_fanin (built)\n";
  return 0;
}
