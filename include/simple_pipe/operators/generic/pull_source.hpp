#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "simple_pipe/media/i_media_reader.hpp"
#include "simple_pipe/operators/generic/source_operator.hpp"

namespace simple_pipe {

class PullSource : public SourceOperator {
 public:
  PullSource(std::string node_id, std::string node_type, nlohmann::json build_init_params);

  bool PullDriven() const override { return true; }
  void Inject(FrameMeta meta) override;
  FrameMeta HandleFrameMeta(const FrameMeta& meta) override;

  void Start() override;
  void Stop() override;

  Result<void> Configure(const nlohmann::json& patch) override;

 protected:
  virtual std::unique_ptr<media::IMediaReader> CreateReader() = 0;

 private:
  void ReaderLoop();

  std::unique_ptr<media::IMediaReader> reader_;
  std::thread read_thread_;
  std::atomic<bool> stop_requested_{false};

  int fps_ = 0;
};

}  // namespace simple_pipe
