#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/pipeline/simple_pipeline.hpp"
#include "test_helper.hpp"

using namespace simple_pipe;
using namespace simple_pipe::test;

struct GraphCase {
  std::string name;
  int frame_count;
};

class PipelineGraphTest : public ::testing::TestWithParam<GraphCase> {};

TEST_P(PipelineGraphTest, BuildStartPushStop) {
  SimplePipeline pipeline(RepoPath("configs/demo/node_defaults").string());
  ASSERT_TRUE((pipeline.BuildFromJson(GraphPath(GetParam().name))).IsSuccess());

  std::atomic<int> received{0};
  pipeline.SetOutputCallback([&](const FrameMeta&) { received.fetch_add(1); });
  ASSERT_TRUE((pipeline.StartPipeline()).IsSuccess());

  for (int i = 0; i < GetParam().frame_count; ++i) {
    FrameInput input;
    input.image = "mock";
    input.frame_index = i;
    ASSERT_TRUE((pipeline.PushFrame(input)).IsSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
  while (received.load() < GetParam().frame_count &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  pipeline.StopPipeline();
  EXPECT_EQ(received.load(), GetParam().frame_count);
}

INSTANTIATE_TEST_SUITE_P(
    AllDemoGraphs, PipelineGraphTest,
    ::testing::Values(GraphCase{"linear", 5}, GraphCase{"five_node", 10},
                      GraphCase{"safety_fanin", 8}),
    [](const ::testing::TestParamInfo<GraphCase>& info) { return info.param.name; });
