#pragma once

#include <utility>

namespace util {

template<typename T>
class auto_restore {
public:
  auto_restore(const auto_restore&) = delete;
  auto_restore(auto_restore&&) = delete;

  auto_restore& operator=(const auto_restore&) = delete;
  auto_restore& operator=(auto_restore&&) = delete;

  auto_restore(T& var)
    : var_(var)
    , old_val_(var) {
  }

  ~auto_restore() {
    var_ = std::move(old_val_);
  }

private:
  T& var_;
  T old_val_;
};

}  // namespace util