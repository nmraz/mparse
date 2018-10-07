#pragma once

#include <utility>

namespace util {

template <typename F>
class finally {
public:
  explicit constexpr finally(F func) : func_(std::move(func)) {}
  ~finally() { func_(); }

private:
  F func_;
};

} // namespace util