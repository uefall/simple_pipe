#pragma once

namespace simple_pipe {

enum class NodeRole { kSrc, kMid, kDes };

enum class DropPolicy { kDropNewest, kKeepLatest };

enum class PipelineState { kUnbuilt, kBuilt, kConfigured, kRunning, kStopped };

}  // namespace simple_pipe
