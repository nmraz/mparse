#pragma once

#include <algorithm>
#include <cstdlib>

namespace mparse {

class source_range {
public:
  constexpr source_range() = default;
  constexpr explicit source_range(std::size_t col) : from_(col), to_(col + 1) {}
  constexpr source_range(std::size_t from, std::size_t to) : from_(from), to_(to) {}

  constexpr std::size_t from() const { return from_; }
  constexpr std::size_t to() const { return to_; }

  constexpr void set_from(std::size_t from) { from_ = from; }
  constexpr void set_to(std::size_t to) { to_ = to; }

  static constexpr source_range merge(const source_range& lhs, const source_range& rhs);

private:
  std::size_t from_ = 0;
  std::size_t to_ = 0;
};

constexpr source_range source_range::merge(const source_range& lhs, const source_range& rhs) {
  return { std::min(lhs.from(), rhs.from()), std::max(lhs.to(), rhs.to()) };
}

}  // namespace mparse