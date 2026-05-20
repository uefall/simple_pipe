#include <gtest/gtest.h>

#include "simple_pipe/builder/pipeline_builder.hpp"
#include "simple_pipe/spec/graph_spec.hpp"
#include "test_helper.hpp"

using namespace simple_pipe;
using namespace simple_pipe::test;

TEST(BuilderTest, LoadLinearGraphAndValidate) {
  auto graph = LoadPipelineGraphSpecFromJson(GraphPath("linear"));
  auto flow = BuildFlowChartPipelineSpecFromGraph(graph);
  auto result = ValidateFlowChartPipelineSpec(flow);
  ASSERT_TRUE(result.IsSuccess());
  EXPECT_EQ(flow.nodes.size(), 3u);
}

TEST(BuilderTest, RejectsMultipleSources) {
  auto graph = LoadPipelineGraphSpecFromJson(GraphPath("linear"));
  graph.nodes.push_back(GraphNodeSpec{"app_src2", "app_src", nlohmann::json::object()});
  auto flow = BuildFlowChartPipelineSpecFromGraph(graph);
  auto result = ValidateFlowChartPipelineSpec(flow);
  ASSERT_FALSE(result.IsSuccess());
}

TEST(BuilderTest, InstantiateLinearPipeline) {
  auto graph = LoadPipelineGraphSpecFromJson(GraphPath("linear"));
  auto flow = BuildFlowChartPipelineSpecFromGraph(graph);
  ASSERT_TRUE(ValidateFlowChartPipelineSpec(flow).IsSuccess());
  auto built = BuildFlowChartPipelineFromSpec(flow);
  EXPECT_EQ(built.nodes.size(), 3u);
  EXPECT_NE(built.source, nullptr);
  EXPECT_NE(built.sink, nullptr);
}
