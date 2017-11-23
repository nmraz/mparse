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

void print_loc(std::string_view source, mparse::source_range where) {
  print_locs(source, { where });
}

void print_locs(std::string_view source, std::initializer_list<mparse::source_range> locs) {
  std::cout << source << '\n';

  mparse::source_range prev_loc;
  for (const auto& loc : locs) {
    std::cout << std::string(loc.from() - prev_loc.to(), ' ')
      << make_marker(loc.to() - loc.from());
    prev_loc = loc;
  }
  std::cout << '\n';
}