#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <thread>
#include <unordered_map>
#include <vector>

#include "simple_pipe/context/frame_meta.hpp"
#include "simple_pipe/enums.hpp"
#include "simple_pipe/result.hpp"

namespace simple_pipe {

class Operator {
 public:
  Operator(std::string node_id, std::string node_type, nlohmann::json build_init_params);
  virtual ~Operator();

  Operator(const Operator&) = delete;
  Operator& operator=(const Operator&) = delete;

  const std::string& node_id() const { return node_id_; }
  const std::string& node_type() const { return node_type_; }

  virtual NodeRole role() const = 0;
  virtual bool emits_downstream() const { return true; }
  virtual Result<void> Configure(const nlohmann::json& patch);

  void AttachTo(const std::vector<std::shared_ptr<Operator>>& upstreams);
  void AddDownstream(const std::shared_ptr<Operator>& downstream);

  void SetInputQueueLimit(int max_size, DropPolicy policy);
  void MetaFlow(FrameMeta meta);
  void Start();
  void Stop();

 protected:
  virtual FrameMeta HandleFrameMeta(const FrameMeta& meta) = 0;
  void PushOutput(FrameMeta meta);

  nlohmann::json build_init_params_;
  std::atomic<bool>& RunningFlag() { return running_; }

 private:
  void EnqueueWithBackpressure(FrameMeta meta);
  void HandleRun();
  void DispatchRun();
  std::optional<FrameMeta> CollectFanIn(FrameMeta meta);
  FrameMeta MergeFanIn(const std::vector<FrameMeta>& parts);

  std::string node_id_;
  std::string node_type_;
  std::vector<std::shared_ptr<Operator>> downstreams_;
  std::vector<std::shared_ptr<Operator>> upstreams_;
  std::unordered_set<std::string> fanin_upstream_ids_;

  std::deque<FrameMeta> in_queue_;
  std::deque<FrameMeta> out_queue_;
  std::mutex in_mutex_;
  std::mutex out_mutex_;
  std::condition_variable in_cv_;
  std::condition_variable out_cv_;

  int max_in_queue_size_ = 8;
  DropPolicy drop_policy_ = DropPolicy::kDropNewest;

  std::atomic<bool> running_{false};
  std::thread handle_thread_;
  std::thread dispatch_thread_;

  std::mutex fanin_mutex_;
  std::unordered_map<int, std::unordered_map<std::string, FrameMeta>> fanin_pending_;
};

}  // namespace simple_pipe
