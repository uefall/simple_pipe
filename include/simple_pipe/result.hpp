#pragma once

#include <optional>
#include <string>

namespace simple_pipe {

template <typename T>
struct Result {
  bool is_ok = false;
  std::optional<T> value;
  std::string error;

  static Result success() {
    Result r;
    r.is_ok = true;
    return r;
  }

  static Result success(T v) {
    Result r;
    r.is_ok = true;
    r.value = std::move(v);
    return r;
  }

  static Result failure(std::string message) {
    Result r;
    r.is_ok = false;
    r.error = std::move(message);
    return r;
  }

  [[nodiscard]] bool IsSuccess() const { return is_ok; }
};

template <>
struct Result<void> {
  bool is_ok = false;
  std::string error;

  static Result success() {
    Result r;
    r.is_ok = true;
    return r;
  }

  static Result failure(std::string message) {
    Result r;
    r.is_ok = false;
    r.error = std::move(message);
    return r;
  }

  [[nodiscard]] bool IsSuccess() const { return is_ok; }
};

}  // namespace simple_pipe
