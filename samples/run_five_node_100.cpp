#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/pipeline/simple_pipeline.hpp"
#include "simple_pipe/spec/runtime_config.hpp"

#ifndef SIMPLE_PIPE_SOURCE_DIR
#define SIMPLE_PIPE_SOURCE_DIR "."
#endif

int main() {
  const std::string root = SIMPLE_PIPE_SOURCE_DIR;
  simple_pipe::SimplePipeline pipeline(root + "/configs/demo/node_defaults");

  if (!(pipeline.BuildFromJson(root + "/configs/demo/graph/five_node.json")).IsSuccess()) {
    std::cerr << "Build failed\n";
    return 1;
  }

  nlohmann::json initial = {
      {"pipeline_params", {{"app_src_in_queue_limit", 16}, {"app_des_in_queue_limit", 16}}},
      {"node_params",
       {{"resize", {{"width", 800}, {"height", 600}}}, {"rules", {{"min_score", 0.5}}}}},
  };
  pipeline.Configure(simple_pipe::PipelineRuntimeConfig::FromJson(initial));

  constexpr int kFrames = 100;
  constexpr int kPatchAt = 50;

  std::atomic<int> received{0};
  std::atomic<int> alerts_before{0};
  std::atomic<int> alerts_after{0};

  pipeline.SetOutputCallback([&](const simple_pipe::FrameMeta& meta) {
    const int idx = meta.frame_index;
    const bool alert =
        meta.extensions.contains("safety") && meta.extensions["safety"].value("alert", false);
    if (idx < kPatchAt && alert) {
      alerts_before.fetch_add(1);
    } else if (idx >= kPatchAt && alert) {
      alerts_after.fetch_add(1);
    }
    received.fetch_add(1);
  });

  pipeline.StartPipeline();
  const auto t0 = std::chrono::steady_clock::now();

  for (int i = 0; i < kFrames; ++i) {
    if (i == kPatchAt) {
      nlohmann::json patch = {
          {"node_params",
           {{"resize", {{"width", 1920}, {"height", 1080}}},
            {"rules", {{"min_score", 0.95}}}}},
      };
      pipeline.Configure(simple_pipe::PipelineRuntimeConfig::FromJson(patch));
      std::cout << "[frame " << i << "] runtime Configure applied\n";
    }
    simple_pipe::FrameInput input;
    input.image = "mock-" + std::to_string(i);
    input.frame_index = i;
    if (!(pipeline.PushFrame(input)).IsSuccess()) {
      std::cerr << "PushFrame failed at " << i << '\n';
      return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
  while (received.load() < kFrames && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  pipeline.StopPipeline();
  const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

  std::cout << "\n=== Summary ===\n"
            << "  frames pushed:   " << kFrames << '\n'
            << "  sink callbacks:  " << received.load() << '\n'
            << "  elapsed (s):     " << elapsed << '\n'
            << "  alerts [0," << kPatchAt << "):  " << alerts_before.load() << '\n'
            << "  alerts [" << kPatchAt << ",100): " << alerts_after.load() << '\n';

  return received.load() == kFrames ? 0 : 1;
}
