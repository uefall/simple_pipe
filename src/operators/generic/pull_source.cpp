#include "simple_pipe/operators/generic/pull_source.hpp"

#include <chrono>
#include <thread>

#include "simple_pipe/log/logger.hpp"
#include "simple_pipe/media/frame_meta_builder.hpp"

namespace simple_pipe {

PullSource::PullSource(std::string node_id, std::string node_type, nlohmann::json build_init_params)
    : SourceOperator(std::move(node_id), std::move(node_type), std::move(build_init_params)) {
  if (build_init_params_.contains("fps")) {
    fps_ = build_init_params_["fps"].get<int>();
  }
}

void PullSource::Inject(FrameMeta meta) {
  if (!RunningFlag().load()) {
    return;
  }
  PushOutput(HandleFrameMeta(meta));
}

FrameMeta PullSource::HandleFrameMeta(const FrameMeta& meta) { return meta.ShallowCopy(); }

Result<void> PullSource::Configure(const nlohmann::json& patch) {
  if (patch.contains("fps")) {
    fps_ = patch["fps"].get<int>();
  }
  return Result<void>::success();
}

void PullSource::Start() {
  Operator::Start();
  stop_requested_.store(false);
  reader_ = CreateReader();
  if (!reader_) {
    log::Error(node_type(), "failed to create reader");
    return;
  }
  auto opened = reader_->Open(build_init_params_);
  if (!opened.IsSuccess()) {
    log::Error(node_type(), opened.error);
    reader_.reset();
    return;
  }
  read_thread_ = std::thread([this] { ReaderLoop(); });
}

void PullSource::Stop() {
  stop_requested_.store(true);
  if (read_thread_.joinable()) {
    read_thread_.join();
  }
  if (reader_) {
    reader_->Close();
    reader_.reset();
  }
  Operator::Stop();
}

void PullSource::ReaderLoop() {
  log::Info(node_type(), "reader loop started for node " + node_id());
  while (!stop_requested_.load() && reader_) {
    auto sample = reader_->Next();
    if (!sample.has_value()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    FrameMeta meta = media::ToFrameMeta(*sample, node_type(), node_id());
    Inject(meta);

    if (meta.IsEos()) {
      log::Info(node_type(), "EOS reached");
      break;
    }

    if (fps_ > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
    }
  }
  log::Info(node_type(), "reader loop stopped");
}

}  // namespace simple_pipe
