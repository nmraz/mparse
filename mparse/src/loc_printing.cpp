#include "loc_printing.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <type_traits>

namespace {

std::string make_marker(std::size_t size) {
  return std::string(size, '~');
}

}  // namespace

void print_loc(mparse::source_range where, std::string_view source) {
  std::cout << source << '\n';

  std::cout << std::string(where.from(), ' ')
    << make_marker(where.to() - where.from()) << '\n';
}

void print_locs(std::initializer_list<mparse::source_range> locs, std::string_view source) {
  std::cout << source << '\n';

  mparse::source_range prev_loc;
  for (const auto& loc : locs) {
    std::cout << std::string(loc.from() - prev_loc.to(), ' ')
      << make_marker(loc.to() - loc.from());
    prev_loc = loc;
  }
  std::cout << '\n';
}