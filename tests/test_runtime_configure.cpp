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

TEST(RuntimeConfigureTest, ConfigureWhileRunningAllowsMorePush) {
  SimplePipeline pipeline;
  ASSERT_TRUE((pipeline.BuildFromJson(GraphPath("five_node"))).IsSuccess());

  nlohmann::json initial = {{"node_params", {{"rules", {{"min_score", 0.5}}}}}};
  ASSERT_TRUE((pipeline.Configure(PipelineRuntimeConfig::FromJson(initial))).IsSuccess());

  std::atomic<int> received{0};
  pipeline.SetOutputCallback([&](const FrameMeta&) { received.fetch_add(1); });
  ASSERT_TRUE((pipeline.StartPipeline()).IsSuccess());

  FrameInput a;
  a.frame_index = 0;
  ASSERT_TRUE((pipeline.PushFrame(a)).IsSuccess());

  nlohmann::json patch = {{"node_params", {{"rules", {{"min_score", 0.99}}}}}};
  ASSERT_TRUE((pipeline.Configure(PipelineRuntimeConfig::FromJson(patch))).IsSuccess());
  EXPECT_TRUE(pipeline.IsRunning());

  FrameInput b;
  b.frame_index = 1;
  ASSERT_TRUE((pipeline.PushFrame(b)).IsSuccess());

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (received.load() < 2 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  pipeline.StopPipeline();
  EXPECT_GE(received.load(), 2);
}
