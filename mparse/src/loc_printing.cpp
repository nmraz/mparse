#include "loc_printing.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

namespace {

std::string make_marker(std::size_t size) {
  return std::string(size, '~');
}

}  // namespace


void print_locs(std::string_view source, util::span<const mparse::source_range> locs) {
  std::cout << source << "\n\x1b[32m";

  mparse::source_range prev_loc;
  for (const auto& loc : locs) {
    assert(prev_loc.to() <= loc.from() && "Overlapping or non-sorted source ranges");
    std::cout << std::string(loc.from() - prev_loc.to(), ' ')
      << make_marker(loc.to() - loc.from());
    prev_loc = loc;
  }
  std::cout << "\n\x1b[0m";
}