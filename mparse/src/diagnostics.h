#pragma once

#include "mparse/source_range.h"
#include "util/span.h"
#include <initializer_list>
#include <string_view>

void print_error(std::string_view type, std::string_view msg);

void print_locs(std::string_view source,
                util::span<const mparse::source_range> underline_locs);

inline void print_locs(
    std::string_view source,
    std::initializer_list<mparse::source_range> underline_locs) {
  print_locs(source,
             util::span(&*underline_locs.begin(), underline_locs.size()));
}

inline void print_loc(std::string_view source, mparse::source_range loc) {
  print_locs(source, {loc});
}

void print_fixit(std::string_view hint, int col);