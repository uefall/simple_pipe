#include "simple_pipe/operators/operator.hpp"

#include <algorithm>
#include <utility>

namespace simple_pipe {

Operator::Operator(std::string node_id, std::string node_type, nlohmann::json build_init_params)
    : build_init_params_(std::move(build_init_params)),
      node_id_(std::move(node_id)),
      node_type_(std::move(node_type)) {}

Operator::~Operator() { Stop(); }

Result<void> Operator::Configure(const nlohmann::json& /*patch*/) { return Result<void>::success(); }

void Operator::AttachTo(const std::vector<std::shared_ptr<Operator>>& upstreams) {
  upstreams_ = upstreams;
  fanin_upstream_ids_.clear();
  for (const auto& up : upstreams_) {
    fanin_upstream_ids_.insert(up->node_id());
  }
}

void Operator::AddDownstream(const std::shared_ptr<Operator>& downstream) {
  if (std::find(downstreams_.begin(), downstreams_.end(), downstream) == downstreams_.end()) {
    downstreams_.push_back(downstream);
  }
}

void Operator::SetInputQueueLimit(int max_size, DropPolicy policy) {
  max_in_queue_size_ = std::max(1, max_size);
  drop_policy_ = policy;
}

void Operator::PushOutput(FrameMeta meta) {
  std::lock_guard<std::mutex> lock(out_mutex_);
  out_queue_.push_back(std::move(meta));
  out_cv_.notify_one();
}

void Operator::MetaFlow(FrameMeta meta) {
  if (!running_.load()) {
    return;
  }
  EnqueueWithBackpressure(std::move(meta));
}

void Operator::EnqueueWithBackpressure(FrameMeta meta) {
  std::lock_guard<std::mutex> lock(in_mutex_);
  if (static_cast<int>(in_queue_.size()) >= max_in_queue_size_) {
    if (drop_policy_ == DropPolicy::kDropNewest) {
      return;
    }
    if (!in_queue_.empty()) {
      in_queue_.pop_front();
    }
  }
  in_queue_.push_back(std::move(meta));
  in_cv_.notify_one();
}

void Operator::Start() {
  if (running_.exchange(true)) {
    return;
  }
  handle_thread_ = std::thread([this] { HandleRun(); });
  dispatch_thread_ = std::thread([this] { DispatchRun(); });
}

void Operator::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(in_mutex_);
    FrameMeta stop;
    stop.frame_index = -1;
    in_queue_.push_back(std::move(stop));
    in_cv_.notify_one();
  }
  {
    std::lock_guard<std::mutex> lock(out_mutex_);
    out_cv_.notify_one();
  }
  if (handle_thread_.joinable()) {
    handle_thread_.join();
  }
  if (dispatch_thread_.joinable()) {
    dispatch_thread_.join();
  }
}

std::optional<FrameMeta> Operator::CollectFanIn(FrameMeta meta) {
  const int idx = meta.frame_index;
  const std::string upstream_id =
      meta.extensions.contains("_upstream_id") ? meta.extensions["_upstream_id"].get<std::string>() : "unknown";

  std::lock_guard<std::mutex> lock(fanin_mutex_);
  auto& bucket = fanin_pending_[idx];
  bucket[upstream_id] = std::move(meta);

  for (const auto& uid : fanin_upstream_ids_) {
    if (bucket.find(uid) == bucket.end()) {
      return std::nullopt;
    }
  }

  std::vector<std::string> sorted_ids(fanin_upstream_ids_.begin(), fanin_upstream_ids_.end());
  std::sort(sorted_ids.begin(), sorted_ids.end());
  std::vector<FrameMeta> parts;
  parts.reserve(sorted_ids.size());
  for (const auto& uid : sorted_ids) {
    parts.push_back(bucket.at(uid));
  }
  fanin_pending_.erase(idx);
  return MergeFanIn(parts);
}

FrameMeta Operator::MergeFanIn(const std::vector<FrameMeta>& parts) {
  FrameMeta base = parts.front().ShallowCopy();
  for (size_t i = 1; i < parts.size(); ++i) {
    const auto& part = parts[i];
    for (auto it = part.images.begin(); it != part.images.end(); ++it) {
      base.images[it.key()] = it.value();
    }
    for (auto it = part.extensions.begin(); it != part.extensions.end(); ++it) {
      const auto key = it.key();
      if (key != "_upstream_id") {
        base.extensions[key] = it.value();
      }
    }
    base.detections.insert(base.detections.end(), part.detections.begin(), part.detections.end());
  }
  return base;
}

void Operator::HandleRun() {
  while (running_.load()) {
    FrameMeta meta;
    {
      std::unique_lock<std::mutex> lock(in_mutex_);
      in_cv_.wait(lock, [this] { return !in_queue_.empty() || !running_.load(); });
      if (!running_.load() && in_queue_.empty()) {
        break;
      }
      if (in_queue_.empty()) {
        continue;
      }
      meta = std::move(in_queue_.front());
      in_queue_.pop_front();
    }

    if (meta.frame_index < 0) {
      break;
    }

    try {
      if (!fanin_upstream_ids_.empty()) {
        auto merged = CollectFanIn(std::move(meta));
        if (!merged.has_value()) {
          continue;
        }
        meta = std::move(*merged);
      }
      FrameMeta out = HandleFrameMeta(meta);
      if (emits_downstream()) {
        std::lock_guard<std::mutex> lock(out_mutex_);
        out_queue_.push_back(std::move(out));
        out_cv_.notify_one();
      }
    } catch (...) {
      continue;
    }
  }
}

void Operator::DispatchRun() {
  while (running_.load()) {
    FrameMeta meta;
    {
      std::unique_lock<std::mutex> lock(out_mutex_);
      out_cv_.wait(lock, [this] { return !out_queue_.empty() || !running_.load(); });
      if (!running_.load() && out_queue_.empty()) {
        break;
      }
      if (out_queue_.empty()) {
        continue;
      }
      meta = std::move(out_queue_.front());
      out_queue_.pop_front();
    }

    for (auto& downstream : downstreams_) {
      FrameMeta copy = meta.ShallowCopy();
      copy.extensions["_upstream_id"] = node_id_;
      downstream->MetaFlow(std::move(copy));
    }
  }
}

}  // namespace simple_pipe
