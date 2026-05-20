#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/pipeline/simple_pipeline.hpp"
#include "simple_pipe/spec/runtime_config.hpp"
#include "test_helper.hpp"

using namespace simple_pipe;
using namespace simple_pipe::test;

namespace {

int RunFrames(SimplePipeline& pipeline, int count, int timeout_ms = 5000) {
  std::atomic<int> received{0};
  pipeline.SetOutputCallback([&](const FrameMeta&) { received.fetch_add(1); });
  if (!pipeline.StartPipeline().IsSuccess()) {
    return -1;
  }

  for (int i = 0; i < count; ++i) {
    FrameInput input;
    input.image = "frame";
    input.frame_index = i;
    if (!pipeline.PushFrame(input).IsSuccess()) {
      pipeline.StopPipeline();
      return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (received.load() < count && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  pipeline.StopPipeline();
  return received.load();
}

}  // namespace

TEST(PipelineTest, LinearGraphProducesOutput) {
  SimplePipeline pipeline(RepoPath("configs/demo/node_defaults").string());
  ASSERT_TRUE(pipeline.BuildFromJson(GraphPath("linear")).IsSuccess());

  nlohmann::json runtime = {
      {"node_params", {{"resize", {{"width", 320}, {"height", 240}}}}},
  };
  ASSERT_TRUE(pipeline.Configure(PipelineRuntimeConfig::FromJson(runtime)).IsSuccess());
  EXPECT_EQ(RunFrames(pipeline, 2), 2);
}

TEST(PipelineTest, SafetyFanInGraphProducesSafetyExtension) {
  SimplePipeline pipeline;
  ASSERT_TRUE(pipeline.BuildFromJson(GraphPath("safety_fanin")).IsSuccess());

  std::atomic<bool> saw_safety{false};
  pipeline.SetOutputCallback([&](const FrameMeta& meta) {
    if (meta.extensions.contains("safety")) {
      saw_safety.store(true);
    }
  });
  ASSERT_TRUE(pipeline.StartPipeline().IsSuccess());
  FrameInput input;
  input.image = "x";
  input.frame_index = 0;
  ASSERT_TRUE(pipeline.PushFrame(input).IsSuccess());

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  pipeline.StopPipeline();
  EXPECT_TRUE(saw_safety.load());
}
