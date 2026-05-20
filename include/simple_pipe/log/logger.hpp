#pragma once

#include <mutex>
#include <string>

namespace simple_pipe::log {

enum class Level { kDebug = 0, kInfo = 1, kWarn = 2, kError = 3 };

class Logger {
 public:
  static Logger& Instance();

  void SetLevel(Level level);
  Level level() const;

  void Log(Level level, const std::string& tag, const std::string& message);

 private:
  Logger() = default;

  mutable std::mutex mutex_;
  Level level_ = Level::kInfo;
};

void Debug(const std::string& tag, const std::string& message);
void Info(const std::string& tag, const std::string& message);
void Warn(const std::string& tag, const std::string& message);
void Error(const std::string& tag, const std::string& message);

}  // namespace simple_pipe::log
