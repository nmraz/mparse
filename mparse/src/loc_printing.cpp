#include "loc_printing.h"

#include <iostream>
#include <string>

void print_loc(mparse::source_range where, std::string_view source) {
  std::cout << source << '\n';

  std::cout << std::string(where.from(), ' ')
    << std::string(where.to() - where.from(), '~') << '\n';
}

void print_locs(const std::vector<mparse::source_range>& locs, std::string_view source) {
  std::cout << source << '\n';

  mparse::source_range prev_loc;
  for (const auto& loc : locs) {
    std::cout << std::string(loc.from() - prev_loc.to(), ' ')
      << std::string(loc.to() - loc.from(), '~');
    prev_loc = loc;
  }
  std::cout << '\n';
}