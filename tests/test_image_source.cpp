#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>

#include "simple_pipe/pipeline/simple_pipeline.hpp"
#include "simple_pipe/spec/graph_spec.hpp"
#include "test_helper.hpp"

using namespace simple_pipe;
using namespace simple_pipe::test;

namespace {

std::filesystem::path WriteTestImages(int count) {
  auto dir = std::filesystem::temp_directory_path() / "simple_pipe_test_images";
  std::filesystem::create_directories(dir);
  for (int i = 0; i < count; ++i) {
    cv::Mat mat(32, 32, CV_8UC3, cv::Scalar(i * 10, 50, 80));
    const auto path = dir / ("frame_" + std::to_string(i) + ".png");
    cv::imwrite(path.string(), mat);
  }
  return dir;
}

}  // namespace

TEST(ImageSourceTest, PullDrivenPipelineReadsAllImages) {
  const auto dir = WriteTestImages(3);
  SimplePipeline pipeline;

  auto graph_path = RepoPath("configs/demo/graph/image_src_linear.json");
  std::ifstream in(graph_path);
  ASSERT_TRUE(in.good());
  nlohmann::json graph;
  in >> graph;
  for (auto& node : graph["nodes"]) {
    if (node["id"] == "img_in") {
      node["init_params"]["path"] = dir.string();
    }
  }
  const auto temp_graph = std::filesystem::temp_directory_path() / "image_src_test_graph.json";
  std::ofstream out(temp_graph);
  out << graph.dump(2);
  out.close();
  ASSERT_TRUE(out.good());

  const auto build_result = pipeline.BuildFromJson(temp_graph.string());
  if (!build_result.IsSuccess()) {
    FAIL() << "Build failed: " << build_result.error;
  }

  std::atomic<int> received{0};
  std::atomic<int> with_buffer{0};
  pipeline.SetOutputCallback([&](const FrameMeta& meta) {
    if (meta.IsEos()) {
      return;
    }
    received.fetch_add(1);
    if (meta.buffer && !meta.buffer->mat.empty()) {
      with_buffer.fetch_add(1);
    }
  });

  ASSERT_TRUE(pipeline.StartPipeline().IsSuccess());

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (received.load() < 3 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  FrameInput blocked;
  blocked.image = "x";
  EXPECT_FALSE(pipeline.PushFrame(blocked).IsSuccess());

  pipeline.StopPipeline();
  EXPECT_EQ(received.load(), 3);
  EXPECT_EQ(with_buffer.load(), 3);
}

TEST(ImageSourceTest, RejectsDualIngressWithAppSrc) {
  nlohmann::json graph = {
      {"nodes",
       {{{"id", "a"}, {"type", "app_src"}},
        {{"id", "b"}, {"type", "image_src"}, {"init_params", {{"path", "/tmp"}}}},
        {{"id", "c"}, {"type", "app_des"}}}},
      {"edges",
       {{{"to", "c"}, {"from", nlohmann::json::array({"a"})}},
        {{"to", "c"}, {"from", nlohmann::json::array({"b"})}}}},
  };

  SimplePipeline pipeline;
  ASSERT_FALSE(pipeline.Build(PipelineGraphSpecFromJson(graph)).IsSuccess());
}
