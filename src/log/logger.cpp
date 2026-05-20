#include "simple_pipe/log/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace simple_pipe::log {

namespace {

const char* LevelName(Level level) {
  switch (level) {
    case Level::kDebug:
      return "DEBUG";
    case Level::kInfo:
      return "INFO";
    case Level::kWarn:
      return "WARN";
    case Level::kError:
      return "ERROR";
  }
  return "UNKNOWN";
}

std::ostream& LevelStream(Level level) {
  return level >= Level::kError ? std::cerr : std::cout;
}

}  // namespace

Logger& Logger::Instance() {
  static Logger instance;
  return instance;
}

void Logger::SetLevel(Level level) { level_ = level; }

Level Logger::level() const { return level_; }

void Logger::Log(Level level, const std::string& tag, const std::string& message) {
  if (level < level_) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  const auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::tm tm_buf{};
  localtime_r(&time, &tm_buf);

  std::ostringstream line;
  line << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0')
       << ms.count() << " [" << LevelName(level) << "] [" << tag << "] " << message;

  std::lock_guard<std::mutex> lock(mutex_);
  LevelStream(level) << line.str() << std::endl;
}

void Debug(const std::string& tag, const std::string& message) {
  Logger::Instance().Log(Level::kDebug, tag, message);
}

void Info(const std::string& tag, const std::string& message) {
  Logger::Instance().Log(Level::kInfo, tag, message);
}

void Warn(const std::string& tag, const std::string& message) {
  Logger::Instance().Log(Level::kWarn, tag, message);
}

void Error(const std::string& tag, const std::string& message) {
  Logger::Instance().Log(Level::kError, tag, message);
}

}  // namespace simple_pipe::log
