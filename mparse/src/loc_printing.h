#pragma once

#include "mparse/source_range.h"
#include <string_view>
#include <vector>

void print_loc(mparse::source_range loc, std::string_view source);
void print_locs(const std::vector<mparse::source_range>& locs, std::string_view source);