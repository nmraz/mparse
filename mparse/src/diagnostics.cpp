#include "diagnostics.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

using namespace std::literals;

namespace {

std::string make_marker(std::size_t size) {
  return std::string(size, '~');
}

std::string make_padding(std::size_t size) {
  return std::string(size, ' ');
}

constexpr std::string_view esc_white = "\x1b[37m";
constexpr std::string_view esc_red = "\x1b[31m";
constexpr std::string_view esc_green = "\x1b[32m";
constexpr std::string_view esc_bold = "\x1b[1m";

constexpr std::string_view esc_reset = "\x1b[0m";

} // namespace


void print_error(std::string_view type, std::string_view msg) {
  std::cout << esc_bold << esc_red << type << " error: " << esc_white << msg
            << esc_reset << "\n\n";
}

void print_locs(std::string_view source,
                util::span<const mparse::source_range> underline_locs) {
  std::cout << source << "\n" << esc_green;

  mparse::source_range prev_underline_loc;
  for (const auto& loc : underline_locs) {
    assert(prev_underline_loc.to() <= loc.from() &&
           "Overlapping or non-sorted source ranges");
    std::cout << make_padding(loc.from() - prev_underline_loc.to())
              << make_marker(loc.to() - loc.from());
    prev_underline_loc = loc;
  }
  std::cout << esc_reset << "\n";
}

void print_fixit(std::string_view hint, std::size_t col) {
  std::cout << esc_green << make_padding(col) << hint << esc_reset << "\n";
}
